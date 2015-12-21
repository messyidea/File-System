#ifndef INIT_H
#define INIT_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "structs.h"
#include "common.h"
#include "shell.h"


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
    buf = (char *)malloc(sizeof(char) * 1000);
}


void create_root_dir() {
    int iid, bid;
    // must 0
    //iid = ialloc();
    set_inode_used(root_inode);
    array_inode[root_inode]->i_uid = ROOTUID;
    array_inode[root_inode]->i_gid = ROOTGID;
    set_inode_mode(root_inode, 0755, true, true);
    array_inode[root_inode]->i_size = (2-1) / 16 + 1;   //2 dir apply 1 block
    array_inode[root_inode]->i_count = 2;               //2 dir . and ..
    bid = balloc();
    printf("bid = %d\n", bid);
    array_inode[root_inode]->i_addr[0] = bid;
    get_single_block(bid);
    p_dir = (struct dir*)(single_block);
    p_dir->inode = root_inode;
    //p_dir->name = ".";
    strcpy(p_dir->name, ".");
    p_dir = (struct dir*)(single_block + sizeof(struct dir));
    p_dir->inode = root_inode;
    //p_dir->name = "..";
    strcpy(p_dir->name, "..");

}

void add_essential_file() {
    int etc_iid, home_iid, root_iid, passwd_iid, bid, i;
    etc_iid = add_file(root_inode, "etc");
    set_inode_dir(etc_iid, root_inode, ROOTUID, ROOTGID, true);
    root_iid = add_file(root_inode, "root");
    set_inode_dir(root_iid, root_inode, ROOTUID, ROOTGID, true);
    home_iid = add_file(root_inode, "home");
    set_inode_dir(home_iid, root_inode, ROOTUID, ROOTGID, true);

    //init user and passwd
    passwd_iid = add_file(etc_iid, "passwd");
    set_inode_file(passwd_iid, root_inode, ROOTUID, ROOTGID, true);
    array_inode[passwd_iid]->i_uid = ROOTUID;
    array_inode[passwd_iid]->i_gid = ROOTGID;
    array_inode[passwd_iid]->i_size = 4;

    // the first block: user_num, group_num and user_group_num
    bid = balloc();
    printf("import bid == %d\n", bid);
    array_inode[passwd_iid]->i_addr[0] = bid;
    get_single_block(bid);
    user_num = (int*)single_block;
    *user_num = 1;
    group_num = (int*)(single_block+4);
    *group_num = 1;
    user_group_num = (int*)(single_block+8);
    *user_group_num = 1;
    max_uid = (int*)(single_block + 12);
    *max_uid = 1;
    max_gid = (int*)(single_block + 16);
    *max_gid = 1;

    //the second block: user
    bid = balloc();
    array_inode[passwd_iid]->i_addr[1] = bid;
    get_single_block(bid);
    for(i = 0; i < MAXUSERNUM; ++i) {
        array_user[i] = (struct user*)(single_block + i * (sizeof(struct user)));
    }
    array_user[0]->uid = 0;
    strcpy(array_user[0]->name, "root");
    strcpy(array_user[0]->passwd, "root");
    puts("初始帐号密码均为root");

    //the third block: group
    bid = balloc();
    array_inode[passwd_iid]->i_addr[2] = bid;
    get_single_block(bid);
    for(i = 0; i < MAXGROUPNUM; ++i) {
        array_group[i] = (struct group*)(single_block + i * (sizeof(struct group)));
    }
    array_group[0]->gid = 0;
    strcpy(array_group[0]->name, "root");

    //the fourth block: user_group
    bid = balloc();
    array_inode[passwd_iid]->i_addr[3] = bid;
    get_single_block(bid);
    for(i = 0; i < MAXUSERGROUPNUM; ++i) {
        array_user_group[i] = (struct user_group*)(single_block + i * (sizeof(struct user_group)));
    }
    array_user_group[0]->uid = 0;
    array_user_group[0]->gid = 0;
}

void init_filesystem() {
    int i,j, tmp, tmp2;
    fp = open("m_filesystem", O_RDWR|O_CREAT, 0777);
    printf("fp == %d\n",fp);
    if(fp < 0) {
        puts("filesystem not existed, new and init a filesystem");
        fp = open("m_filesystem", O_RDWR|O_CREAT, 0777);
        p_filesys = (struct filsys*)filesystem;
        p_filesys->s_isize = 512;
        p_filesys->s_fsize = 3 + 64 + 1024;
        p_filesys->s_nfree = 99;
        p_filesys->s_ninode = 99;
        // init stack
        for(i = 0; i < 100; ++i) {
            p_filesys->s_free[i] = i;
            // the id 0 inode is used
            p_filesys->s_inode[i] = i + 1;
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

        //create "/"
        create_root_dir();

        // add etc passwd to save passwd
        add_essential_file();

        //printf("sizeof p_filesys = %d\n", sizeof(*p_filesys));
        puts("start write");
        write(fp, filesystem, FILESYSTEMSIZE);

        //read(fp1, filesystem, FILESYSTEMSIZE);
        //printf("%d\n", p_filesys->s_isize);
        //close(fp1);
    } else {
        read(fp, filesystem, FILESYSTEMSIZE);
        p_filesys = (struct filesys*)filesystem;
        //printf("sisize == %d\n", p_filesys->s_isize);
        //printf("fsize == %d\n", p_filesys->s_fsize);
        // get node
        for(i = 0; i < 512; ++i) {
            array_inode[i] = (struct inode*)(filesystem + 512*3 + 64*i);
        }
        //get block
        p_used_block = (struct used_block *)(filesystem + 512*2);


        //debug_show_dir(0);

        //get passwd
        /*
        tmp = array_inode[root_inode]->i_addr[0];
        //printf("tmp == %d\n",tmp);
        get_single_block(tmp);
        p_dir = (struct dir*)(single_block + 2*(sizeof(struct dir)));
        printf("name == %s\n", p_dir->name);
        tmp = p_dir->inode;
        tmp = array_inode[tmp]->i_addr[0];
        get_single_block(tmp);
        p_dir = (struct dir*)(single_block + 2*(sizeof(struct dir)));
        tmp = p_dir->inode;
        //printf("name == %s\n", p_dir->name);
        //now tmp is the inode of passwd

        printf("tmp == %d\n",tmp);
        printf("from function = %d\n", get_inode_from_path("/etc/passwd"));
        */
        tmp = get_inode_from_path("/etc/passwd");
        tmp2 = array_inode[tmp]->i_addr[0];
        //printf("important bid2 == %d\n",tmp2);
        get_single_block(tmp2);

        user_num = (int*)single_block;
        printf("user_num == %d\n", *user_num);
        group_num = (int*)(single_block + 4);
        user_group_num = (int*)(single_block + 8);
        max_uid = (int*)(single_block + 12);
        max_gid = (int*)(single_block + 16);
        tmp2 = array_inode[tmp]->i_addr[1];
        get_single_block(tmp2);
        for(i = 0; i < MAXUSERNUM; ++i) {
            array_user[i] = (struct user*)(single_block + i * sizeof(struct user));
        }
        tmp2 = array_inode[tmp]->i_addr[2];
        get_single_block(tmp2);
        for(i = 0; i < MAXGROUPNUM; ++i) {
            array_group[i] = (struct group*)(single_block + i * (sizeof(struct group)));
        }
        tmp2 = array_inode[tmp]->i_addr[3];
        get_single_block(tmp2);
        for(i = 0; i < MAXUSERGROUPNUM; ++i) {
            array_user_group[i] = (struct user_group*)(single_block + i * (sizeof(struct user_group)));
        }
        puts("init success");
    }
}
void init() {
    //data init
    init_data();

    //filesystem init
    init_filesystem();
}



#endif // INIT_H
