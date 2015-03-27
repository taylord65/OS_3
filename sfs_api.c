#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include "disk_emu.h"
#include "sfs_api.h"

super_block superblock;

int current_dir_position;


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
//		inodes[45] = testinode;

		memcpy((void *)inodetable_buffer, (const void *) &inodes, sizeof(inodes));		
		write_blocks(1, 15, inodetable_buffer);

		memset(root_dir,0,sizeof(root_dir));


		memcpy((void *)directory_buffer, (const void *) &root_dir, sizeof(root_dir));
		write_blocks(16, 5, directory_buffer);		

		memset(free_bitmap, 0, sizeof(free_bitmap));
		free_bitmap[BLOCK_SIZE-1]=1; //The free bitmap occupies the last block
		free_bitmap[0]=255; //First 8 bits set to 1 (superblock occupied, blocks 1-15 occupied with inodes)
		free_bitmap[1]=255; //rest of inodes
		free_bitmap[2]=248;//The root directory takes blocks 16-21
		write_blocks(NUM_BLOCKS-1,1,free_bitmap);		 

		memset(fd_table,0, sizeof(fd_table)); //file descriptor table is saved in memory not on disk
		
		current_dir_position = 0;


	} else {

		init_disk("Dotsikas_Taylor_sfs", BLOCK_SIZE, NUM_BLOCKS); 

		read_blocks(0, 1, (void *)&superblock );

		read_blocks(1, 15, inodetable_buffer);

		//Load the inode entries from disk into the inodes[] 
		int i;

		for(i=0;i<NUM_INODES;i++){

			memcpy((void *)&(inodes[i]),(const void *)(inodetable_buffer+i*( sizeof(inode)/4)), sizeof(inode)); 
		//	printf("INODE %d has pointer 1: %d\n", i, inodes[i].pointers[0]);
		

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

		current_dir_position = 0;
	}

}


int sfs_fopen(char *name){

	//printf("%s \n", name);

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

	//A new directory entry has now been created along with its inode, write the inodes[] back to the disk. Overwrite everything

	int inodes_buffer[15 * BLOCK_SIZE];	
	memset(inodes_buffer,0, sizeof(inodes_buffer));		
	memcpy((void *)inodes_buffer, (const void *) &inodes, sizeof(inodes));		
	write_blocks(1, 15, inodes_buffer);

	//Write the root_dir[] back to the disk. Overwrite everything

	int dir_buffer[5 * BLOCK_SIZE];
	memset(dir_buffer,0,sizeof(dir_buffer));
	memcpy((void *)dir_buffer, (const void *) &root_dir, sizeof(root_dir));
	write_blocks(16, 5, dir_buffer);	


	return dir_index; //Return the new files index in the directory

} 

int sfs_fclose(int fileID){

	//If it is open close it

	if(fd_table[fileID].opened == 1){

		fd_table[fileID].opened = 0;
		return 0;
	
	}

} 


int find_free_block(void){
//If it is free set it to occupied, then return the block number
	int i;
	for(i=0;i<BLOCK_SIZE;i++){
		if((free_bitmap[i]&128)==0){
			free_bitmap[i]&128 == 1;
			return i*8+1;
		}
		else if((free_bitmap[i]&64)==0){
			free_bitmap[i]&64 == 1;			
			return i*8+2;
		}
		else if((free_bitmap[i]&32)==0){
			free_bitmap[i]&32 == 1;			
			return i*8+3;
		}
		else if((free_bitmap[i]&16)==0){
			free_bitmap[i]&16 == 1;			
			return i*8+4;
		}
		else if((free_bitmap[i]&8)==0){
			free_bitmap[i]&8 == 1;			
			return i*8+5;
		}
		else if((free_bitmap[i]&4)==0){
			free_bitmap[i]&4 == 1;			
			return i*8+6;
		}
		else if((free_bitmap[i]&2)==0){
			free_bitmap[i]&2 == 1;			
			return i*8+7;
		}
		else if((free_bitmap[i]&1)==0){
			free_bitmap[i]&1 == 1;						
			return i*8+8;
		}
	}
	printf("The disk is full\n");
}


int sfs_fwrite(int fileID, const char *buf, int length){
	//at end write inodes back to disk, write data to disk
	//can only write inside the file, at the end of the file, or write to an empty file

	char data_buffer[BLOCK_SIZE];


	if(length < 0){
		return -1;
		//error in length size
	}

	inode selected_inode = inodes[root_dir[fileID].inode_number];
	int current_position = fd_table[fileID].rw_ptr;


	if(selected_inode.pointers[0] == 0){
	//This is an empty file
	//write to a block starting at the beginning, cannot assume the data is continuous across sequential blocks

		memset(data_buffer,0,sizeof(data_buffer));

		int blocks_to_write = length/BLOCK_SIZE;
		int free_block;

		if(blocks_to_write == 0){
			//can fit everything in one block
			free_block = find_free_block();
			memcpy((void *)&(data_buffer),(const void *)(buf),length);
			write_blocks(free_block,1,data_buffer); 
			inodes[root_dir[fileID].inode_number].pointers[0] = free_block;

		}
		else{
			//need to write multiple blocks

			if(length%BLOCK_SIZE > 0){
				//need to write some of another block so use a full one
				blocks_to_write++;
			}

			int blocks_array_to_write[blocks_to_write];
			int new_free_block, i,j;
			int length_to_write, remaining_bytes;

			remaining_bytes = length;

			for(i=0;i<blocks_to_write;i++){

				new_free_block = find_free_block();
				blocks_array_to_write[i] = new_free_block;
				
				//Update the inode
				inodes[root_dir[fileID].inode_number].pointers[i] = blocks_array_to_write[i];

			}

				for(j=0;j<blocks_to_write;j++){

				length_to_write = remaining_bytes/BLOCK_SIZE;

				if(length_to_write != 0){
					//Write a full block
					memcpy((void *)&(data_buffer),(const void *)(buf+(sizeof(data_buffer))*j),sizeof(data_buffer)); 
					write_blocks(blocks_array_to_write[j],1,data_buffer);

					remaining_bytes = remaining_bytes - BLOCK_SIZE;

				}
				else {
					//Write the last one
					memcpy((void *)&(data_buffer),(const void *)(buf+(sizeof(data_buffer))*j),remaining_bytes); 
					write_blocks(blocks_array_to_write[j],1,data_buffer);
				}

				memset(data_buffer,0,sizeof(data_buffer));

			}

		}//end write multiple new blocks



	}
	else {
	//Not an empty file

	int start_block = 0;
	int start_block_index =0;

	int j;
	for(j=0;j<NUM_INODE_POINTERS-1;j++){

		//does the current_position lie inside one of the inode's data blocks
		if( current_position >= selected_inode.pointers[j-1]*BLOCK_SIZE && current_position <= selected_inode.pointers[j]*BLOCK_SIZE){
			start_block = selected_inode.pointers[j];
			start_block_index = j;
			break;
		}

	}	

	if(start_block == 0){
		//The pointer does not lie inside the file
		printf("Pointer does not lie inside the file\n");
		return -1;
	}

	int rw_offset_from_start_block = current_position - (start_block-1)*BLOCK_SIZE;

	if((rw_offset_from_start_block + length) < BLOCK_SIZE){
		//Only need to write one block, the start_block, it must be loaded into a buffer, the buf needs to be added at the offset

		memset(data_buffer,0,sizeof(data_buffer));
		read_blocks(start_block,1,data_buffer);

		memcpy((void *)&(data_buffer[rw_offset_from_start_block]),(const void *)buf, length); 
		write_blocks(start_block,1,data_buffer);


	}
	else{
		//Need to write the current + more free blocks
		//Load current block into buffer and do the first part of the length into it

		memset(data_buffer,0,sizeof(data_buffer));
		read_blocks(start_block,1,data_buffer);

		memcpy((void *)&(data_buffer[rw_offset_from_start_block]),(const void *)buf, (BLOCK_SIZE - rw_offset_from_start_block) ); 
		write_blocks(start_block,1,data_buffer);
		//Current block is written

		int remaining_bytes = length - (BLOCK_SIZE - rw_offset_from_start_block);

		//How many blocks does it take to fit remaining_bytes

		int blocks_to_write = remaining_bytes/BLOCK_SIZE;

		if(blocks_to_write == 0){
			if(remaining_bytes%BLOCK_SIZE > 0){
				blocks_to_write = 1;
				//only need to write some of a block, but will use a full one
				//overwrite the next block in the inode pointers or find a free one if there is no next one

				if(selected_inode.pointers[start_block_index+1] != 0){
					//The next block already exists

					memset(data_buffer,0,sizeof(data_buffer));
					read_blocks(start_block+1,1,data_buffer);

					memcpy((void *)&(data_buffer),(const void *)(buf+(BLOCK_SIZE - rw_offset_from_start_block)), remaining_bytes); 
					write_blocks(start_block+1,1,data_buffer);					
				}
				else{

					//need to find 1 new block since there is not another datablock pointed to in the inode

					int freeblock = find_free_block();

					memset(data_buffer,0,sizeof(data_buffer));
					read_blocks(freeblock,1,data_buffer);

					memcpy((void *)&(data_buffer),(const void *)(buf+(BLOCK_SIZE - rw_offset_from_start_block)), remaining_bytes); 
					write_blocks(freeblock,1,data_buffer);	

					//Add datablock to inode
					inodes[root_dir[fileID].inode_number].pointers[start_block_index+1] = freeblock;

						int inodes_buffer[15 * BLOCK_SIZE];	
						memset(inodes_buffer,0, sizeof(inodes_buffer));		
						memcpy((void *)inodes_buffer, (const void *) &inodes, sizeof(inodes));		
						write_blocks(1, 15, inodes_buffer);

				}

			}
		} 
		else {
			//More than one block needs to be written with remaining_bytes, check if there are inode pointers already that need to be overwritten or find new

			if(remaining_bytes%BLOCK_SIZE > 0){
				//need to write some of another block so use a full one
				blocks_to_write++;
			}

			memset(data_buffer,0,sizeof(data_buffer));


			int blocks_array_to_write[blocks_to_write];
			int new_free_block, i,j;
			int length_to_write;

			for(i=0;i<blocks_to_write;i++){
				if(selected_inode.pointers[start_block_index+1+i] != 0){
					//Check if next pointer is free
					blocks_array_to_write[i] = selected_inode.pointers[start_block_index+1+i];
				}
				else {
					//If it is not, find a new block
					new_free_block = find_free_block();
					blocks_array_to_write[i] = new_free_block;
				}	
				//Update the inode
				inodes[root_dir[fileID].inode_number].pointers[start_block_index+1+i] = blocks_array_to_write[i];

			}

			//Write the remaining_bytes data to all the entries of blocks_array_to_write
			//read in every block, then write

			for(j=0;j<blocks_to_write;j++){

				read_blocks(blocks_array_to_write[j],1,data_buffer); //Read what is currently in that block

				length_to_write = remaining_bytes/BLOCK_SIZE;

				if(length_to_write != 0){
					//Write a full block
					memcpy((void *)&(data_buffer),(const void *)(buf+(BLOCK_SIZE - rw_offset_from_start_block)+(j*sizeof(data_buffer))),sizeof(data_buffer)); 
					write_blocks(blocks_array_to_write[j],1,data_buffer);

					remaining_bytes = remaining_bytes - BLOCK_SIZE;

				}
				else {
					//Write the last one
					memcpy((void *)&(data_buffer),(const void *)(buf+(BLOCK_SIZE - rw_offset_from_start_block)+(j*sizeof(data_buffer))),remaining_bytes); 
					write_blocks(blocks_array_to_write[j],1,data_buffer);
				}

				memset(data_buffer,0,sizeof(data_buffer));

			}

		}		

	}


	} //end not an empty file

	//write the inodes back to the disk
	int inodes_buffer[15 * BLOCK_SIZE];	
	memset(inodes_buffer,0, sizeof(inodes_buffer));		
	memcpy((void *)inodes_buffer, (const void *) &inodes, sizeof(inodes));		
	write_blocks(1, 15, inodes_buffer);

	//write freeblock bitmap back to the disk
	write_blocks(NUM_BLOCKS-1,1,free_bitmap);		 


}

int sfs_fread(int fileID, char *buf, int length){

	int num_blocks_to_read = length/BLOCK_SIZE;

	int inode_index = root_dir[fileID].inode_number;
	inode selected_inode = inodes[inode_index];

	char data_buffer[(NUM_INODE_POINTERS-1)*BLOCK_SIZE]; //this will hold all the data for the selected file
	memset(data_buffer,0,sizeof(data_buffer));

	int i;

	for(i=0;i<num_blocks_to_read;i++){

		if(selected_inode.pointers[i]==0){
			//Inode has no data
			return -1;
		}

		read_blocks(selected_inode.pointers[i],1,data_buffer+i*BLOCK_SIZE);
	}

	memcpy(buf, data_buffer, length); //copy the data into buf

}

int sfs_fseek(int fileID, int offset){

	//Set the selected file's pointer to the offset value
	fd_table[fileID].rw_ptr = offset;

}

int sfs_remove(char *file){

}

int sfs_get_next_filename(char* filename){

	//Need to create a new directory table that contains all the non zero entries

	int i, j, k, num_non_zero_entries;

	for(i=0;i<MAXFILES;i++){
		if(root_dir[i].inode_number!=0){
			num_non_zero_entries++;
		}
	}

	directory_entry non_zero_entries[num_non_zero_entries];

	k = 0;
	for(j=0;j<MAXFILES;j++){
		if(root_dir[j].inode_number!=0){
			non_zero_entries[k] = root_dir[j];
			k++;
		}
	}

	if(current_dir_position==(num_non_zero_entries-1)){
		//current position is last file
		current_dir_position=0;
		return 0;
	}
	else {
		//Get the next file and put it in filename, return non zero
		filename = non_zero_entries[current_dir_position++].file_name;
		return 1;
	}

}

int sfs_GetFileSize(const char* path){
	//Get the base of the path (file name) look in the root_dir to find its corresponding inode and use the size property
	//not handling indirect inode
	char *base=basename((char *)path);
	int index, i,j,k;

	for(i=0;i<MAXFILES;i++){
		if(strncmp(root_dir[i].file_name, base, MAXFILENAME)==0){
			index = root_dir[i].inode_number;
		}
	}
	int filesize;

	inode selected_inode = inodes[index];

	int num_data_blocks; //how many blocks it uses
	num_data_blocks = 0;

	for(j=0;j<NUM_INODE_POINTERS-1;j++){
		if(selected_inode.pointers[j]!=0){
			num_data_blocks++;
		}
	}

	int the_pointers[num_data_blocks];

	for(k=0;k<num_data_blocks;k++){
		if(selected_inode.pointers[k]!=0){
			the_pointers[k] = selected_inode.pointers[k];
		}

	}

	int last_block = the_pointers[num_data_blocks-1];
	int full_blocks = num_data_blocks - 1;

	char buffer[BLOCK_SIZE];

	read_blocks(last_block,1,buffer);

	int buf_pointer = BLOCK_SIZE -1;

	while(buffer[buf_pointer]==0){
		buf_pointer--; //Decrement the buf pointer until it points to the end of the data
	}

	buf_pointer++; //Now the buf_pointer contains the amount of bytes in the last block

	filesize = buf_pointer + full_blocks*BLOCK_SIZE;

	inodes[index].size = filesize; //Update the inodes size property

	return filesize;


}
