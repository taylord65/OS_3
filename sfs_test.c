#include <stdio.h>
#include "sfs_api.h"

int main(void){
	printf("Disk has been created\n");
	mksfs(0);
	return 0;
}