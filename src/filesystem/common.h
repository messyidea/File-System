#ifndef COMMON_H
#define COMMON_H
#include <structs.h>

inline void set_inode_used(int id) {
    array_inode[id].i_mode |= IALLOC;
}

inline void set_inode_unused(int id) {
    array_inode[id].i_mode = 0;
}

inline bool is_inode_used(int id) {
    return (array_inode[id].i_mode & IALLOC) == IALLOC;
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
    return (array_inode[id].i_mode & IFDIR) == IFDIR;
}

void set_inode_mode(int id, int mode, bool is_dir) {
    array_inode[id].i_mode = 0;
    if(is_dir)
        array_inode[id].i_mode |= IFDIR;
    array_inode[id].i_mode += mode;
}

#endif
