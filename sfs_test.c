#include <stdio.h>
#include "sfs_api.h"

int main(void){
	printf("Disk has been created\n");
	mksfs(1);
	return 0;
}