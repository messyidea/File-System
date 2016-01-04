#include <stdio.h>
#include <stdlib.h>
#include "structs.h"
#include "init.h"
#include "login.h"
#include <time.h>

/*
super_node 1024字节
inode 64字节
block 512字节
1 block = 8 inode
(2 block) (1 block) (512 inode = 64 block) (1024 block)
*/
int main()
{
    /*
    printf("Hello world!\n");
    printf("sizeof int = %d\n", sizeof(int));
    printf("sizeof filsys = %d\n", sizeof(struct filsys));
    printf("sizeof inode = %d \n", sizeof(struct inode));
    //filesystem = (char *) malloc (512 * (3 + 64 + 1024));
    //printf("sizeof filesystem = %d", sizeof(filesystem));
    */
    struct inode a;
    printf("size =  %d\n", sizeof(struct inode));
    printf("size int = %d\n", sizeof(int));
    printf("size char = %d\n", sizeof(char));
    printf("sizeof ctime = %d\n", sizeof(time_t));
    init();
    //debug_show_dir(0);

    if(!login()) exit(0);

    shell();

    return 0;
}
