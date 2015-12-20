#ifndef COMMON_H
#define COMMON_H
#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

inline void set_inode_used(int id) {
    array_inode[id]->i_mode |= IALLOC;
}

inline void set_inode_unused(int id) {
    array_inode[id]->i_mode = 0;
}

inline bool is_inode_used(int id) {
    return (array_inode[id]->i_mode & IALLOC) == IALLOC;
}

int ialloc() {
    int i;
    if(p_filesys->s_ninode < 0) {
        for(i = 0; i < INODENUM; ++i) {
            if(!is_inode_used(i)) {
                p_filesys->s_inode[++p_filesys->s_ninode] = i;
                if(p_filesys->s_ninode == 99) break;
            }
        }
        if(p_filesys->s_ninode < 0) return -1;
    }

    //alloc
    set_inode_used(p_filesys->s_inode[p_filesys->s_ninode--]);
    return p_filesys->s_inode[p_filesys->s_ninode + 1];
}

void set_block_used(int id) {
    int i, j;
    i = id / 32;
    j = id - i * 32;
    p_used_block->u[i] |= 1 << j;
}

void set_block_unused(int id) {
    int i, j;
    i = id / 32;
    j = id - i * 32;
    p_used_block->u[i] &= ~(1 << j);
}

bool is_block_used(int id) {
    int i, j;
    i = id / 32;
    j = id - i * 32;
    return  (p_used_block->u[i] & (1 << j)) == (1 << j);
}

int balloc() {
    int i;
    if(p_filesys->s_nfree < 0) {
        for(i = 0; i < BLOCKNUM; ++i) {
            if(!is_block_used(i)) {
                p_filesys->s_free[++(p_filesys->s_nfree)] = i;
                if(p_filesys->s_nfree == 99) break;
            }
        }
        if(p_filesys->s_nfree < 0) return -1;
    }

    //alloc
    set_inode_used(p_filesys->s_inode[p_filesys->s_ninode--]);
    return p_filesys->s_inode[p_filesys->s_ninode + 1];
}

bool is_inode_dir(int id) {
    return (array_inode[id]->i_mode & IFDIR) == IFDIR;
}

void set_inode_mode(int id, int mode, bool is_dir) {
    array_inode[id]->i_mode = 0;
    if(is_dir)
        array_inode[id]->i_mode |= IFDIR;
    array_inode[id]->i_mode += mode;
}

void get_single_block(int id) {
    single_block = (char*)(filesystem + 512*(2 + 1 + 64 + id));
}

int add_file(int id, char name[14]) {
    int before, after, place, iid, bid;
    place = array_inode[id]->i_count - (array_inode[id]->i_count / 16) * 16;
    before = array_inode[id]->i_count ++;
    after = array_inode[id]->i_count;
    before = (before-1) / 16 + 1;
    after = (after-1) / 16 + 1;
    if(before == after) {
        //do not need to balloc
        get_single_block(array_inode[id]->i_addr[array_inode[id]->i_size - 1]);
        p_dir = (struct dir*)(single_block + sizeof(struct dir) * (place));
        iid = ialloc();
        p_dir->inode = iid;
        //p_dir->name = name;
        strcpy(p_dir->name, name);
        return iid;
    } else {
        if(array_inode[id]->i_size == 8) {
            puts("can not create more dir");
            return -1;
        }
        bid = balloc();
        array_inode[id]->i_addr[ array_inode[id]->i_size ++ ] = bid;
        get_single_block(bid);
        p_dir =(struct dir*)(single_block);
        iid = ialloc();
        p_dir->inode = iid;
        //p_dir->name = name;
        strncpy(p_dir->name, name, strlen(name));
        return iid;
    }
    return -1;
}

bool set_inode_dir(int id, int uid, int gid) {
    int bid;
    array_inode[id]->i_uid = uid;
    array_inode[id]->i_gid = gid;
    set_inode_mode(id, 0755, true);
    array_inode[id]->i_size = (2-1) / 16 + 1;   //2 dir apply 1 block
    array_inode[id]->i_count = 2;               //2 dir . and ..
    bid = balloc();
    if(bid == -1) {
        puts("no more block, create dir failed");
        return false;
    }
    array_inode[id]->i_addr[0] = bid;
    get_single_block(bid);
    p_dir = (struct dir*)(single_block);
    p_dir->inode = id;
    //p_dir->name = ".";
    strcpy(p_dir->name, ".");
    p_dir = (struct dir*)(single_block + sizeof(struct dir));
    p_dir->inode = id;
    //p_dir->name = "..";
    strcpy(p_dir->name, "..");
}

#endif
