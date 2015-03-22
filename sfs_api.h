#define MAXFILENAME 16
#define MAXFILEEXTENSION 3
#define NUM_BLOCKS 496
#define BLOCK_SIZE 512 
#define NUM_INODE_POINTERS 13
#define MAXFILES 10
#define MAXFILESIZE 10

//On disk data structures of the file system

typedef struct{
	char mode; 
	int link_cnt;
	int uid;
	int gid;
	int size;
	int *pointers[NUM_INODE_POINTERS];
} inode;

typedef struct{
	inode inodes[MAXFILES+1]; //Set how many files you can have, 1 needs to be the inode that points to the root directory
} inode_table;

typedef struct
{
	int list[NUM_BLOCKS]; //Keep track of freespace in the SFS
} freeblocklist;

typedef struct 
{
	int magic_number;
	int blocksize; //Bytes in block 
	int file_system_size; //Number of blocks
	int inode_table_length; //Number of blocks
	int root_directory; //The inode that is the root directory (a number)
} super_block;

typedef struct 
{
	int inode_number;
	char file_name[MAXFILENAME+MAXFILEEXTENSION];
} directory_entry;


root_dir directory_entry[MAXFILES];


typedef struct
{
    int opened;
    int wr_ptr;
    int rd_ptr;
} file_descriptor;



int mksfs(int fresh);
int sfs_fopen(char *name);
int sfs_fclose(int fileID);
int sfs_fwrite(int fileID, const char *buf, int length);
int sfs_fread(int fileID, char *buf, int length);
int sfs_fseek(int fileID, int offset);
int sfs_remove(char *file);
int sfs_get_next_filename(char* filename);
int sfs_GetFileSize(const char* path);
