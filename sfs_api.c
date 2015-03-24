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
				printf("Directory entry %d file name: %s\n", (k + block_num*21), root_dir[k + block_num*21].file_name);

				
				}
				block_num++;
				memset(directory_buffer,0, sizeof(directory_buffer));
			}

		}	


	}

}


int sfs_fopen(char *name){
//opens a file and returns the index on the file descriptor table
/*
	int i;
	int fileID;

	//Look through the root directory to find the fileID that coresponds to the name
	for(i=0;i < MAXFILES;i++)
	{
		if(strcmp(name,root.table[i].file_name) == 0){
			fileID = i;
		}
	}
	
	//Check if the file is already open 
	if(root.table[fileID ].fd.opened == 1)
	{
		printf("Error file '%s' already open!\n", name);
		return -1;
	}
	else
	{
		//It will be opened now
		root.table[fileID ].fd.opened = 1;
		return 0;
	}

	return fileID;
	*/
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
