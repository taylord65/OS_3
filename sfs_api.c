#include <stdio.h>
#include <string.h>
#include "disk_emu.h"
#include "sfs_api.h"

super_block superblock;

int mksfs(int fresh){

	int superblock_buffer[BLOCK_SIZE];
	memset(superblock_buffer,0,BLOCK_SIZE);

	int inodetable_buffer[15 * BLOCK_SIZE];
	memset(inodetable_buffer,0, sizeof(inodetable_buffer));	

	char directory_buffer[5 * BLOCK_SIZE];
	memset(directory_buffer,0, sizeof(directory_buffer));

	if (fresh) {
	
		init_fresh_disk("Dotsikas_Taylor_sfs", BLOCK_SIZE, NUM_BLOCKS);

		super_block superblock={
			.magic_number=0xAABB0005,
			.blocksize=BLOCK_SIZE,
			.file_system_size=NUM_BLOCKS,
			.inode_table_length=NUM_BLOCKS,
			.root_directory=0 
		}; 

		memcpy((void *)superblock_buffer, (const void *) &superblock, sizeof(super_block));
		write_blocks(0, 1, superblock_buffer); //The superblock is the first block on the disk

		inode rootinode={
			.mode=0777, 
			.link_cnt=1,
			.uid=0,
			.gid=0,
			.size=0,
			.pointers={16,17,18,19,20,0,0,0,0,0,0,0,0}
		};


		memset(inodes, 0, sizeof(inodes)); //Set inode table to 0, sizeof(inodes) gives the size of the array in bytes

		inodes[superblock.root_directory] = rootinode; //write the rootinode into the table

		memcpy((void *)inodetable_buffer, (const void *) &inodes, sizeof(inodes));		
		write_blocks(1, 15, inodetable_buffer);

		memset(root_dir,0,sizeof(root_dir));

/*		directory_entry dirtest={
			.inode_number=10,
			.file_name="thisistest"
		};

		root_dir[5] = dirtest;*/

		memcpy((void *)directory_buffer, (const void *) &root_dir, sizeof(root_dir));
		write_blocks(16, 5, directory_buffer);		

		memset(free_bitmap, 0, sizeof(free_bitmap));
		free_bitmap[BLOCK_SIZE-1]=1; //The free bitmap occupies the last block
		free_bitmap[0]=255; //First 8 bits set to 1 (superblock occupied, blocks 1-15 occupied with inodes)
		free_bitmap[1]=255; //rest of inodes
		free_bitmap[2]=248;//The root directory takes blocks 16-21
		write_blocks(NUM_BLOCKS-1,1,free_bitmap);		 

		memset(fd_table,0, sizeof(fd_table)); //file descriptor table is saved in memory not on disk


	} else {

		init_disk("Dotsikas_Taylor_sfs", BLOCK_SIZE, NUM_BLOCKS); 

		read_blocks(0, 1, (void *)&superblock );

		read_blocks(1, 15, inodetable_buffer);

		//Load the inode entries from disk into the inodes[] 
		int i;

		for(i=0;i<NUM_INODES;i++){

			memcpy((void *)&(inodes[i]),(const void *)(inodetable_buffer+i*( sizeof(inode)/4)), sizeof(inode)); 

		}

		memset(free_bitmap, 0, sizeof(free_bitmap)); //Clear free_bitmap in memory 
		read_blocks(NUM_BLOCKS-1,1,free_bitmap); //Read from disk back into free bitmap

		memset(fd_table,0, sizeof(fd_table)); //file descriptor table is saved in memory not on disk		

		inode rootinode = inodes[superblock.root_directory];

		int j;
		int block_num;
		
		//Load all of the directory entries from disk into root_dir[]
		for(j=0;j<NUM_INODE_POINTERS;j++){

			if(rootinode.pointers[j] != 0){

				read_blocks(rootinode.pointers[j],1,directory_buffer);
				int k;
				//Each block can contain 21, 24 byte directory entries
				for (k=0;k<21;k++){

				memcpy((void *)&(root_dir[k + block_num*21]), (const void *)(directory_buffer+k*( sizeof(directory_entry) )), sizeof(directory_entry)); 
				//printf("Directory entry %d file name: %s\n", (k + block_num*21), root_dir[k + block_num*21].file_name);

				}
				block_num++;
				memset(directory_buffer,0, sizeof(directory_buffer));
			}

		}	


	}

}


int sfs_fopen(char *name){

	int i;
	for(i=0;i<MAXFILES;i++){

		if(strncmp(root_dir[i].file_name, name, MAXFILENAME) == 0){
			//The file exists in the directory

			if(fd_table[i].opened=1){
				return -1; //The file is already open
			}

			fd_table[i].opened = 1; //Open the file

			inode file_inode = inodes[root_dir[i].inode_number]; //Load an inode into memory that contains the pointers to the data for the file

			int j;
			for(j=0;j<NUM_INODE_POINTERS;j++){
				if(file_inode.pointers[j]==0){
					//File pointer needs to be set to end of file, need to find the last block

					char block_buffer[BLOCK_SIZE];
					read_blocks(file_inode.pointers[j-1], 1, block_buffer);
					//The block buffer now holds the block that holds the ending data of the file

					int f_pointer = BLOCK_SIZE - 1; 

					while(block_buffer[f_pointer] == 0){
						f_pointer--; //Decrement the pointer until it points to the last byte
					}

					fd_table[i].rw_ptr = (f_pointer + 1) + (file_inode.pointers[j-2])*BLOCK_SIZE; 
					//The byte number of the end of the file beginning from the start of the disk is now saved in the pointer


					return i; //Return the index of the file
				}
			}
			//All pointers are non zero, the file's inode uses an indirect pointer

			//-------TODO--------

		}
	}

	//The File does not exist and needs to be created

	int k;
	int dir_index;

	for(k=0;k<MAXFILES;k++){
		if(root_dir[k].inode_number==0){
			//This is an unused directory entry
			dir_index = k;
			break;
		}
	}

	directory_entry new_entry;
	strcpy(new_entry.file_name, name);


	int l;
	int new_inode_index; //the index for the inode that will contain data for the new file

	for(l=1;l<NUM_INODES;l++){
		if(inodes[l].link_cnt==0){
			new_inode_index=l;
			break;
		}
	}

	inode new_inode={
		.mode=0777, 
		.link_cnt=1,
		.uid=0,
		.gid=0,
		.size=0,
		.pointers={0,0,0,0,0,0,0,0,0,0,0,0,0}
	};

	inodes[new_inode_index] = new_inode;
	new_entry.inode_number = new_inode_index;
	root_dir[dir_index] = new_entry;

	//A new directory entry has now been created along with its inode, write the inodes[] back to the disk

	int inodes_buffer[15 * BLOCK_SIZE];	
	memset(inodes_buffer,0, sizeof(inodes_buffer));		
	memcpy((void *)inodes_buffer, (const void *) &inodes, sizeof(inodes));		
	write_blocks(1, 15, inodes_buffer);

	//Write the root_dir[] back to the disk

	int dir_buffer[5 * BLOCK_SIZE];
	memset(dir_buffer,0,sizeof(dir_buffer));
	memcpy((void *)dir_buffer, (const void *) &root_dir, sizeof(root_dir));
	write_blocks(16, 5, dir_buffer);		


	return dir_index; //Return the new files index in the directory

} 

int sfs_fclose(int fileID){

} 

int sfs_fwrite(int fileID, const char *buf, int length){

}

int sfs_fread(int fileID, char *buf, int length){

}

int sfs_fseek(int fileID, int offset){

}

int sfs_remove(char *file){

}

int sfs_get_next_filename(char* filename){

}

int sfs_GetFileSize(const char* path){

}
