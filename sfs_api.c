#include <stdio.h>
#include "disk_emu.h"
#include "sfs_api.h"

super_block superblock;
freeblocklist freebl;
root_dir root; 
inode_table inodetable;

int mksfs(int fresh){

	if (fresh) {
		//Create the file system from scratch
		init_fresh_disk("mysfs", BLOCK_SIZE, NUM_BLOCKS);

		super_block superblock={
			.magic=0xAABB0005,
			.blocksize=BLOCK_SIZE,
			.file_system_size=NUM_BLOCKS,
			.inode_table_length=NUM_BLOCKS,
			.root_directory=ROOTDIR_INODE_NUMBER
		}; 

		int i;

		for(i = 0; i < NUM_BLOCKS; i++) 
		{
			freebl.list[i] = 1; //In the beginning every block is free
		}

		write_blocks(0, 1, (void *)&superblock); //The superblock is the first block on the disk
		write_blocks(1, 100, (void *)&inodetable);
		write_blocks(100, 20000, (void *)&root); //Root directory cannot grow larger than max file size, which is 20000 blocks.

		write_blocks(NUM_BLOCKS-1, 1, (void *)&freebl); 



	} else {
		//System is opened from the disk
		init_disk("mysfs", BLOCK_SIZE, NUM_BLOCKS); 

		read_blocks(0, 1, (void *)&superblock); 
		read_blocks(1, 20000, (void *)&root);

		read_blocks(NUM_BLOCKS-51, 1, (void *)&freebl);


	}

}


int sfs_fopen(char *name){
//opens a file and returns the index on the file descriptor table

	int i;
	int root_directory_size = sizeof(root.table)/sizeof(root.table[0]);
	int fileID;

	//Look through the root directory to find the fileID that coresponds to the name
	for(i=0;i<root_directory_size;i++)
	{
		if(name == root.table[i].file_name){
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
		root.table[fileID ].fd.opened = 1;
		return 0;
	}

	return fileID;
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
