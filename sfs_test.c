#include <stdio.h>
#include "sfs_api.h"

int main(void){
	printf("Disk has been created\n");
	mksfs(0);

	char s[10] = "hello";
	sfs_fopen(s);
	return 0;
}