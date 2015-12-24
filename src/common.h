#ifndef COMMON_H
#define COMMON_H
#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// 设置inodeid为已使用
inline void set_inode_used(int id) {
    array_inode[id]->i_mode |= IALLOC;
}
// 设置inodeid为未使用
inline void set_inode_unused(int id) {
    array_inode[id]->i_mode = 0;
}
// inodeid是否已使用
inline bool is_inode_used(int id) {
    return (array_inode[id]->i_mode & IALLOC) == IALLOC;
}
// 分配一个空闲的inode
int ialloc() {
    //puts("ialloc");
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
// 设置block为使用
void set_block_used(int id) {
    int i, j;
    i = id / 32;
    j = id - i * 32;
    p_used_block->u[i] |= 1 << j;
}
// 设置block为未使用
void set_block_unused(int id) {
    int i, j;
    i = id / 32;
    j = id - i * 32;
    p_used_block->u[i] &= ~(1 << j);
}
// 判断block是否已使用
bool is_block_used(int id) {
    int i, j;
    i = id / 32;
    j = id - i * 32;
    return  (p_used_block->u[i] & (1 << j)) == (1 << j);
}
// 分配一个空闲的block
int balloc() {
    //printf("balloc\n");
    int i;
    if(p_filesys->s_nfree < 0) {
        //puts("lllllllllllllllllllllllllllll");
        for(i = 0; i < BLOCKNUM; ++i) {
            if(!is_block_used(i)) {
                p_filesys->s_free[++(p_filesys->s_nfree)] = i;
                if(p_filesys->s_nfree == 99) break;
            }
        }
        if(p_filesys->s_nfree < 0) {
            puts("fail");
            exit(0);
            return -1;
        }
    }
    set_block_used(p_filesys->s_free[p_filesys->s_nfree --]);

    return p_filesys->s_free[p_filesys->s_nfree + 1];
}

// inode对应的是否是文件夹
bool is_inode_dir(int id) {
    return (array_inode[id]->i_mode & IFDIR) == IFDIR;
}

// 设置inode的mode
void set_inode_mode(int id, int mode, bool is_dir, bool is_important) {
    array_inode[id]->i_mode = 0;
    if(is_dir == true)
        array_inode[id]->i_mode |= IFDIR;
    if(is_important == true)
        array_inode[id]->i_mode |= IIMPORTANT;
    array_inode[id]->i_mode += mode;
}

// 获取block数据到single_block
void get_single_block(int id) {
    single_block = (char*)(filesystem + 512*(2 + 1 + 64 + id));
}
// 在文件夹id中添加一个inode，名字叫name
int add_file(int id, char *name) {
    int i, len;
    int before, after, place, iid, bid;
    place = array_inode[id]->i_count - ((array_inode[id]->i_count - 1) / 16) * 16;
    before = array_inode[id]->i_count ++;
    after = array_inode[id]->i_count;
    before = (before-1) / 16 + 1;
    after = (after-1) / 16 + 1;
    namebuf[0] = 0;
    strcpy(namebuf, name);
    if(before == after) {
        //do not need to balloc
        get_single_block(array_inode[id]->i_addr[array_inode[id]->i_size - 1]);
        p_dir = (struct dir*)(single_block + sizeof(struct dir) * (place));
        iid = ialloc();
        p_dir->inode = iid;
        strcpy(p_dir->name, namebuf);
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
        strcpy(p_dir->name, namebuf);
        return iid;
    }
    return -1;
}
// 设置inode为目录
bool set_inode_dir(int id, int pid, int uid, int gid, bool is_important) {
    int bid;
    array_inode[id]->i_uid = uid;
    array_inode[id]->i_gid = gid;
    array_inode[id]->i_pid = pid;
    set_inode_mode(id, 0755, true, is_important);
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
    strcpy(p_dir->name, ".");
    p_dir = (struct dir*)(single_block + sizeof(struct dir));
    p_dir->inode = pid;
    strcpy(p_dir->name, "..");
}

// 设置inode为文件
bool set_inode_file(int id, int pid, int uid, int gid, bool is_important) {
    int bid;
    array_inode[id]->i_uid = uid;
    array_inode[id]->i_gid = gid;
    array_inode[id]->i_pid = pid;
    set_inode_mode(id, 0755, false, is_important);
    array_inode[id]->i_size = 0;

}
// inode是否是目录
bool is_dir(int id) {
    return (array_inode[id]->i_mode & IFDIR) == IFDIR;
}
// 从文件夹id中找文件path，找到返回inode，没找到返回-1
int find_inode_from_single_path(int id, char *path) {
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
        if(strcmp(p_dir->name, path) == 0) return p_dir->inode;
    }
    return -1;

}
// 根据path找到inode，找到返回inode，没找到返回-1
int get_inode_from_path(char* path) {
    if(strlen(path) == 0) return curr_inode;
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

// debug用，显示dir
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
// 获取当前用户名
inline char* get_username() {
    return array_user[curr_user]->name;
}
// 获取文件夹id中的文件id2的名字
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
        if(p_dir->inode == id2 ) return p_dir->name;
    }
    return "";
}

// 通过id返回文件的绝对路径
char* get_dirname(int id) {
    if(id == root_inode) return "/";
    int sta[100], pos = 0, tid;
    sta[pos ++] = id;
    while(id != 0) {
        id = find_inode_from_single_path(id, "..");
        sta[pos ++] = id;
    }
    strcpy(buf, "/");
    pos --;
    tid = 0;
    while(--pos >= 0) {
        tid = sta[pos + 1];
        strcat(buf, get_singlepath_from_inode(tid, sta[pos]));
        if(pos != 0) strcat(buf, "/");
    }
    return buf;
}

// 文件夹是否为空
bool is_dir_empty(int id) {
    return array_inode[id]->i_count == 2;
}

// 删除文件夹pid中的饿文件id
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
        if(array_inode[pid]->i_count % 16 == 0) {
            array_inode[pid]->i_size --;
            set_block_unused(array_inode[pid]->i_addr[array_inode[pid]->i_size + 1]);
        }
        return ;
    }

    for(i = 0; i < num - 1; ++i) {
        tmpid = array_inode[pid]->i_addr[i];
        get_single_block(tmpid);
        for(j = 0;j < 16; ++j) {
            p_dir = (struct dir*)(single_block + j * (sizeof(struct dir)));
            if(p_dir->inode == id) {
                p_dir->inode = tp->inode;
                strcpy(p_dir->name, tp->name);
                if(array_inode[pid]->i_count % 16 == 0) {
                    array_inode[pid]->i_size --;
                    set_block_unused(array_inode[pid]->i_addr[array_inode[pid]->i_size + 1]);
                }
                return ;
            }
        }
    }
    i = num - 1;
    tmpid = array_inode[pid]->i_addr[i];
    get_single_block(tmpid);
    for(j = 0;j < lastnum; ++j) {
        p_dir = (struct dir*)(single_block + j * (sizeof(struct dir)));
        if(p_dir->inode == id ) {
            p_dir->inode = tp->inode;
            strcpy(p_dir->name, tp->name);
            if(array_inode[pid]->i_count % 16 == 0) {
                array_inode[pid]->i_size --;
                set_block_unused(array_inode[pid]->i_addr[array_inode[pid]->i_size + 1]);
            }
            return ;
        }
    }
}

// 删除空目录
void remove_empty_dir(int id) {
    int bid = array_inode[id]->i_addr[0];
    set_inode_unused(id);
    set_block_unused(bid);
}

// 判断路径是否符合要求
bool check_path(char *path) {
    int len = strlen(path);
    int i;
    for(i = 0; i < len - 1; ++i) {
        if(path[i] == '/' && path[i+1] == '/') {
            return false;
        }
    }
    return true;
}

// 用户uid是否有文件iid的w权限
bool have_authority(int uid, int iid, char w) {
    if(uid == ROOTUID) {
        return  true;
    }
    int mode;
    mode = array_inode[iid]->i_mode;
    switch(w) {
        case 'w':
            if(array_inode[iid]->i_uid != uid) {
                return (mode & (1<<1)) == 1<<1;
            } else {
                return (mode & (1<<7)) == 1<<7;
            }
            break;
        case 'r':
            if(array_inode[iid]->i_uid != uid) {
                return (mode & (1<<2)) == 1<<2;
            } else {
                return (mode & (1<<8)) == 1<<8;
            }
            break;
        case 'x':
            if(array_inode[iid]->i_uid != uid) {
                return (mode & (1<<0)) == 1<<0;
            } else {
                return (mode & (1<<6)) == 1<<6;
            }
            break;
        default:
            return false;
            break;
    }
}

// 文件或文件夹id是否是important的
bool is_important(int id) {
    return (array_inode[id]->i_mode & IIMPORTANT) == IIMPORTANT;
}
// 更改文件夹id中的文件id2的名字为dirname
void change_dir_name(int id, int id2, char*dirname) {
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
            if(p_dir->inode == id2) strcpy(p_dir->name, dirname);
        }
    }
    i = num - 1;
    tmpid = array_inode[id]->i_addr[i];
    get_single_block(tmpid);
    for(j = 0;j < lastnum; ++j) {
        p_dir = (struct dir*)(single_block + j * (sizeof(struct dir)));
        if(p_dir->inode == id2 ) strcpy(p_dir->name, dirname);
    }
}
// 保存内存中的数据到文件
void save() {
    lseek(fp,0,SEEK_SET);
    write(fp, filesystem, FILESYSTEMSIZE);
}
// inode是否是large的
bool is_inode_large(int id) {
    return  (array_inode[id]->i_mode & ILARG) == ILARG;
}
// 设置inode为large
void set_inode_large(int id) {
    array_inode[id]->i_mode |= ILARG;
}

#endif
