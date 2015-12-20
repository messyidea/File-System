#ifndef COMMAND_H
#define COMMAND_H
#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void command_ls(int id) {
    int i, j, tmpid;
    int lastnum, num;
    lastnum = array_inode[id]->i_count;
    lastnum -= (lastnum-1) / 16 * 16;
    num = array_inode[id]->i_size;
    //前num-1个是满的
    for(i = 0; i < num - 1; ++i) {
        tmpid = array_inode[id]->i_addr[i];
        get_single_block(tmpid);
        for(j = 0;j < 16; ++j) {
            p_dir = (struct dir*)(single_block + j * (sizeof(struct dir)));
            printf("%s     \n", p_dir->name);
        }
    }
    i = num - 1;
    tmpid = array_inode[id]->i_addr[i];
    get_single_block(tmpid);
    for(j = 0;j < lastnum; ++j) {
        p_dir = (struct dir*)(single_block + j * (sizeof(struct dir)));
        //printf("-----%d %d\n", p_dir->inode, id2);
        printf("%s     \n", p_dir->name);
    }
    return "";
}

void command_cd(char* path) {
    puts("do cd");
    int iid = get_inode_from_path(path);
    if(iid < 0) {
        puts("文件不存在");
        return ;
    }
    if(!is_dir(iid)) {
        puts("打开的不是目录");
        return ;
    }
    curr_inode = iid;
}

int command_mkdir(char* path) {
    int tmpid, pos = 0, pos2, pid = 0, iid;
    int len;
    if(path[0] == '/') { tmpid = root_inode; pos ++; }
    else tmpid = curr_inode;
    len = strlen(path);
    if(path[len - 1] == '/') len --;
    pid = -1;
    while(pos < len) {
        pos2 = 0;
        while(pos < len && path[pos] != '/') {
            single_pathbuf[pos2 ++] = path[pos++];
        }
        single_pathbuf[pos2] = 0;
        pid = tmpid;
        tmpid = find_inode_from_single_path(tmpid, single_pathbuf);
        pos ++;
        if(tmpid == -1 && pos < len) return -1;
    }
    if(pid < 0) return -1;
    iid = add_file(pid, single_pathbuf);
    set_inode_dir(iid, pid, curr_user, curr_user);

    return 1;
}

int command_rmdir(char* path) {
    int pid;
    int iid = get_inode_from_path(path);
    if(iid < 0) {
        puts("目录不存在！");
        return -1;
    }
    if(!is_dir(iid)) {
        puts("要删除的是文件！");
        return -2;
    }
    if(!is_dir_empty(iid)) {
        puts("目录非空");
        return -3;
    }
    //quanxian

    pid = find_inode_from_single_path(iid, "..");
    remove_dir_use_inode(pid, iid);


}





#endif // COMMAND_H
