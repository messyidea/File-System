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
        strcpy(p_dir->name, name);
        return iid;
    }
    return -1;
}

bool set_inode_dir(int id, int pid, int uid, int gid) {
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
    p_dir->inode = pid;
    //p_dir->name = "..";
    strcpy(p_dir->name, "..");
}

bool is_dir(int id) {
    return (array_inode[id]->i_mode & IFDIR) == IFDIR;
}

int find_inode_from_single_path(int id, char *path) {
    //printf("find inode from single path %d\n", id);
    int i, j, tmpid;
    if(!is_dir(id)) return -1;
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
            if(strcmp(p_dir->name, path) == 0) return p_dir->inode;
        }
    }
    i = num - 1;
    tmpid = array_inode[id]->i_addr[i];
    get_single_block(tmpid);
    for(j = 0;j < lastnum; ++j) {
        p_dir = (struct dir*)(single_block + j * (sizeof(struct dir)));
        //printf("%s\n", p_dir->name);
        if(strcmp(p_dir->name, path) == 0) return p_dir->inode;
    }
    return -1;

}

int get_inode_from_path(char* path) {
    if(strcmp(path, "/") == 0) return root_inode;
    int tmpid, pos = 0, pos2;
    int len;
    if(path[0] == '/') { tmpid = root_inode; pos ++; }
    else tmpid = curr_inode;
    len = strlen(path);
    if(path[len - 1] == '/') len --;
    while(pos < len) {
        pos2 = 0;
        while(pos < len && path[pos] != '/') {
            single_pathbuf[pos2 ++] = path[pos++];
        }
        single_pathbuf[pos2] = 0;
        tmpid = find_inode_from_single_path(tmpid, single_pathbuf);
        if(tmpid == -1) return -1;
        pos ++;
    }
    return tmpid;
}

void debug_show_dir(int id) {
    puts("==========================");
    int i, num;
    num = array_inode[id]->i_count;
    printf("count = %d\n",num);
    get_single_block(array_inode[id]->i_addr[0]);
    for(i = 0; i<num; ++i) {
        p_dir = (struct dir*)(single_block + i * sizeof(struct dir));
        printf("%s\n", p_dir->name);
    }
    puts("============================");
}

inline char* get_username() {
    return array_user[curr_user]->name;
}

char* get_singlepath_from_inode(int id, int id2) {
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
            if(p_dir->inode == id2) return p_dir->name;
        }
    }
    i = num - 1;
    tmpid = array_inode[id]->i_addr[i];
    get_single_block(tmpid);
    for(j = 0;j < lastnum; ++j) {
        p_dir = (struct dir*)(single_block + j * (sizeof(struct dir)));
        //printf("-----%d %d\n", p_dir->inode, id2);
        if(p_dir->inode == id2 ) return p_dir->name;
    }
    return "";
}

/*  */
char* get_dirname(int id) {
    if(id == root_inode) return "/";
    int sta[100], pos = 0, tid;
    sta[pos ++] = id;
    while(id != 0) {
        id = find_inode_from_single_path(id, "..");
        sta[pos ++] = id;
        //printf("id = %d\n", id);
    }
    strcpy(buf, "/");
    pos --;
    tid = 0;
    while(--pos >= 0) {
        tid = sta[pos + 1];
        //printf("%d %d %s\n", tid, sta[pos], get_singlepath_from_inode(tid, sta[pos]));
        strcat(buf, get_singlepath_from_inode(tid, sta[pos]));
        if(pos != 0) strcat(buf, "/");
    }
    return buf;
}

bool is_dir_empty(int id) {
    return array_inode[id]->i_count == 2;
}

void remove_dir_use_inode(int pid, int id) {
    int lastnum, num, bid, i, j, tmpid;
    lastnum = array_inode[pid]->i_count;
    lastnum -= (lastnum-1) / 16 * 16;
    num = array_inode[pid]->i_size;
    struct dir* tp;
    bid = array_inode[pid]->i_addr[num - 1];
    get_single_block(bid);
    tp = (struct dir*)(single_block + (lastnum-1) * (sizeof(struct dir)));
    array_inode[pid]->i_count --;
    if(tp->inode == id) {
        if(array_inode[pid]->i_count % 16 == 0) array_inode[pid]->i_size --;
        return ;
    }

    for(i = 0; i < num - 1; ++i) {
        tmpid = array_inode[id]->i_addr[i];
        get_single_block(tmpid);
        for(j = 0;j < 16; ++j) {
            p_dir = (struct dir*)(single_block + j * (sizeof(struct dir)));
            if(p_dir->inode == id) {
                p_dir->inode = tp->inode;
                strcpy(p_dir->name, tp->name);
                if(array_inode[pid]->i_count % 16 == 0) array_inode[pid]->i_size --;
                return ;
            }
        }
    }
    i = num - 1;
    tmpid = array_inode[id]->i_addr[i];
    get_single_block(tmpid);
    for(j = 0;j < lastnum; ++j) {
        p_dir = (struct dir*)(single_block + j * (sizeof(struct dir)));
        //printf("-----%d %d\n", p_dir->inode, id2);
        if(p_dir->inode == id ) {
            p_dir->inode = tp->inode;
            strcpy(p_dir->name, tp->name);
            if(array_inode[pid]->i_count % 16 == 0) array_inode[pid]->i_size --;
            return ;
        }
    }
}

#endif
