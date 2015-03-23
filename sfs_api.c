#include <stdio.h>
#include <string.h>
#include "disk_emu.h"
#include "sfs_api.h"

super_block superblock;

int mksfs(int fresh){

	int buffer[BLOCK_SIZE];
	memset(buffer,0,BLOCK_SIZE);

	int inodetable_buffer[25 * BLOCK_SIZE];
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

		memcpy((void *)buffer, (const void *) &superblock, sizeof(super_block));
		write_blocks(0, 1, buffer); //The superblock is the first block on the disk

		inode rootinode={
			.mode=0777, 
			.link_cnt=1,
			.uid=0,
			.gid=0,
			.size=0,
			.pointers={26,0,0,0,0,0,0,0,0,0,0,0,0}
		};

		memset(inodes, 0, sizeof(inodes)); //Set inode table to 0, sizeof(inodes) gives the size of the array in bytes

		//printf("Size of inode array entry %d\n", sizeof(inodes[0]));

		inodes[superblock.root_directory] = rootinode; //write the rootinode into the table
		memcpy((void *)inodetable_buffer, (const void *) &inodes, sizeof(inodes));		
		write_blocks(1, 25, inodetable_buffer);


		memset(root_dir,0,sizeof(root_dir));
		memcpy((void *)directory_buffer, (const void *) &root_dir, sizeof(root_dir));
		write_blocks(26, 5, directory_buffer); 


	} else {

		init_disk("mysfs", BLOCK_SIZE, NUM_BLOCKS); 

		read_blocks(0, 1, (void *)&superblock );

		printf("%x \n", superblock.magic_number);

		read_blocks(1, 25, inodetable_buffer);

		//Load the inodetable_buffer into the inodes[] array
		int i;
		for(i=0;i<NUM_INODES;i++){

			memcpy((void *)&(inodes[i]),(const void *)(inodetable_buffer+i*(sizeof(inodes[i]))), sizeof(inodes[i])); 
			printf("INODE %d has pointer 1: %d\n", i, inodes[i].pointers[0]);

		}

		printf("Root inode Mode is : %d \n", inodes[superblock.root_directory].mode);

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
