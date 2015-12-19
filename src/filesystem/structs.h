#ifndef STRUCTS_H
#define STRUCTS_H

struct filsys {
    int s_isize;
    int s_fsize;
    int s_nfree;
    int s_free[100];
    int s_ninode;
    int s_inode[100];
    char s_flock;
    char s_ilock;
    char s_fmod;
    char s_ronly;
    char s_time[2];
    int pad[50];
};

struct inode {
    int i_mode;
    char i_nlink;
    char i_uid;
    char i_gid;
    char i_size0;
    char *i_size1;
    int i_addr[8];
    int i_lastr;
    int i_count;
};


struct used_block {
    int u[32];
};

char *filesystem;
char *single_block;
struct inode* array_inode[512];
struct used_block* p_used_block;
struct filsys *p_filesys;
int fp;


#define FILESYSTEMSIZE (512 * (3 + 64 + 1024))
#define INODENUM 512
#define BLOCKNUM 1024

/* MODES */
#define IALLOC 0100000
#define IFMT 060000
#define IFDIR 040000
#define IFCHR 020000
#define IEXEC 0100

#endif // STRUCTS
