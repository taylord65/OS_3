#include <stdio.h>
#include "disk_emu.h"
#include "sfs_api.h"
#include <string.h>

super_block superblock;
freeblocklist freebl;

root_dir root; 

inode_table inodetable;
inode rootinode;

int mksfs(int fresh){

	if (fresh) {
		//Create the file system from scratch
		init_fresh_disk("mysfs", BLOCK_SIZE, NUM_BLOCKS);

		super_block superblock={
			.magic_number=0xAABB0005,
			.blocksize=BLOCK_SIZE,
			.file_system_size=NUM_BLOCKS,
			.inode_table_length=NUM_BLOCKS,
			.root_directory=0 
		}; 

		memset(&root,0,sizeof(root));

		inode rootinode={
			.mode=0777, 
			.link_cnt=1,
			.uid=0,
			.gid=0,
			.size=0,
			.pointers={26,0,0,0,0,0,0,0,0,0,0,0,0}
		};

		memcpy(&inodetable[superblock.root_directory], &rootinode, sizeof(rootinode));

		/*int i;

		for(i = 0; i < NUM_BLOCKS; i++) 
		{
			freebl.list[i] = 1; //In the beginning every block is free
		}*/

		write_blocks(0, 1, (void *)&superblock); //The superblock is the first block on the disk
		write_blocks(1, 25, (void *)&inodetable);
		write_blocks(26, 5, (void *)&root); //Root directory cannot grow larger than max file size, which is 20000 blocks.

		write_blocks(NUM_BLOCKS-1, 1, (void *)&freebl); 

		return(0);


	} else {

		//System is opened from the disk
		init_disk("mysfs", BLOCK_SIZE, NUM_BLOCKS); 
		read_blocks(0, 1, (void*)&superblock);
		printf("%x \n", superblock.magic_number);

		read_blocks(1, 25, (void*)&inodetable);
		printf("Root inode Mode is : %c \n", rootinode.mode);

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
