#define MAXFILENAME 16
#define MAXFILEEXTENSION 3
#define NUM_BLOCKS 4096
#define BLOCK_SIZE 512 
#define NUM_INODE_POINTERS 13
#define MAXFILES 99
#define MAXFILESIZE 10
#define NUM_INODES 100

//On disk data structures of the file system

typedef struct{
	int magic_number;
	int blocksize; //Bytes in block 
	int file_system_size; //Number of blocks
	int inode_table_length; //Number of blocks
	int root_directory; //The inode that is the root directory (a number)
} super_block;

typedef struct{
	int mode; 
	int link_cnt;
	int uid;
	int gid;
	int size;
	int pointers[NUM_INODE_POINTERS];
} inode;

inode inodes[NUM_INODES];

unsigned char free_bitmap[BLOCK_SIZE];

typedef struct{
	int inode_number;
	char file_name[MAXFILENAME+MAXFILEEXTENSION];
} directory_entry;


directory_entry root_dir[MAXFILES];

typedef struct{
    int opened;
    int rw_ptr;
} file_descriptor;

file_descriptor fd_table[MAXFILES];

int mksfs(int fresh);
int sfs_fopen(char *name);
int sfs_fclose(int fileID);
int sfs_fwrite(int fileID, const char *buf, int length);
int sfs_fread(int fileID, char *buf, int length);
int sfs_fseek(int fileID, int offset);
int sfs_remove(char *file);
int sfs_get_next_filename(char* filename);
int sfs_GetFileSize(const char* path);
