#include <stdio.h>
#include "disk_emu.h"
#include "sfs_api.h"

super_block superblock;
freeblocklist freebl;
root_dir root; 

int mksfs(int fresh){

	if (fresh) {
		//Create the file system from scratch
		init_fresh_disk("mysfs", BLOCK_SIZE, NUM_BLOCKS); 

		int i;

		for(i = 0; i < NUM_BLOCKS; i++) 
		{
			freebl.list[i] = 1; //In the beginning every block is free
		}

		write_blocks(0, 1, (void *)&superblock); //The superblock is the first block on the disk
		write_blocks(1, 20000, (void *)&root); //Root directory cannot grow larger than max file size, which is 20000 blocks.

		write_blocks(NUM_BLOCKS-51, 1, (void *)&freebl); 



	} else {
		//System is opened from the disk
		init_disk("mysfs", BLOCK_SIZE, NUM_BLOCKS); 

		read_blocks(0, 1, (void *)&superblock); 
		read_blocks(1, 20000, (void *)&root);

		read_blocks(NUM_BLOCKS-51, 1, (void *)&freebl);


	}

}


int sfs_fopen(char *name){

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
