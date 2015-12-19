#ifndef INIT_H
#define INIT_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "structs.h"

void init_data() {
/*
super_node 1024字节
inode 64字节
block 512字节
1 block = 8 inode
(2 block) (1 block) (512 inode = 64 block) (1024 block)
*/
    filesystem = (char *) malloc (FILESYSTEMSIZE);
    // memset?
}

void init_filesystem() {
    int i,j;
    fp = open("m_filesystem", O_RDONLY);
    printf("fp == %d\n",fp);
    if(fp < 0) {
        puts("filesystem not existed, new and init a filesystem");
        fp = open("m_filesystem", O_RDWR|O_CREAT);
        p_filesys = (struct filsys*)filesystem;
        p_filesys->s_isize = 512;
        p_filesys->s_fsize = 3 + 64 + 1024;
        p_filesys->s_nfree = 100;
        p_filesys->s_ninode = 100;
        // init stack
        for(i = 0; i < 100; ++i) {
            p_filesys->s_free[i] = i;
            p_filesys->s_inode[i] = i;
        }
        // clear inode
        for(i = 0; i < 512; ++i) {
            array_inode[i] = (struct inode*)(filesystem + 512*3 + 64*i);
            array_inode[i]->i_mode = 0;
        }
        // clear block
        p_used_block = (struct used_block *)(filesystem + 512*2);
        for(i = 0; i < 32; ++i) p_used_block->u[i] = 0;
        //finish init , start create default file




        //printf("sizeof p_filesys = %d\n", sizeof(*p_filesys));
        write(fp, p_filesys, sizeof(*p_filesys));
        //read(fp1, p_filesys, sizeof(p_filesys));
        //printf("%d\n", p_filesys->s_isize);
        //close(fp1);
    } else {
        read(fp, filesystem, FILESYSTEMSIZE);
        p_filesys = (struct filesys*)filesystem;
        // get node
        for(i = 0; i < 512; ++i) {
            array_inode[i] = (struct inode*)(filesystem + 512*3 + 64*i);
        }
        //get block
        p_used_block = (struct used_block *)(filesystem + 512*2);
    }
}
void init() {
    //data init
    init_data();

    //filesystem init
    init_filesystem();
}


#endif // INIT_H
