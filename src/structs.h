#ifndef STRUCTS_H
#define STRUCTS_H
#include <time.h>

// 超级块
struct filsys {
    int s_isize;
    int s_fsize;
    int s_nfree;
    int s_free[100];
    int s_ninode;
    int s_inode[100];
};
// inode
struct inode {
    int i_mode;
    char i_uid;
    char i_gid;
    int i_size;
    int i_addr[8];
    int i_count;
    int i_pid;
    //time_t i_createtime;
    time_t i_changetime;
};


struct dir {
    int inode;
    char name[14];
};

// 块表
struct used_block {
    int u[32];
};

struct user {
    int uid;
    char name[14];
    char passwd[16];
};

struct group {
    int gid;
    char name[14];
};

struct user_group {
    int uid;
    int gid;
};

// c里面没有bool类型
typedef enum {
    false, true
}bool;


char *filesystem;
char *inputbuf;
char *filesbuf;
char *single_block;
char *namebuf;
char *output_pathbuf;
struct inode* array_inode[512];
struct user* array_user[16];
struct group* array_group[32];
struct user_group* array_user_group[64];
struct used_block* p_used_block;
struct filsys *p_filesys;
struct dir *p_dir;
int *user_num, *group_num, *user_group_num;
int *max_uid, *max_gid;
int fp;
const int root_inode = 0;
int curr_inode;
int curr_user;
char *buf;
char usernamebuf[100];
char passwordbuf[100];
char path[100][100];
int pathtop;
char pathbuf[1000];
char single_pathbuf[100];
char commandbuf[10][100];
int command_num;



#define ROOTUID 0
#define ROOTGID 0

#define FILESYSTEMSIZE (512 * (3 + 64 + 1024))
#define INODENUM 512
#define BLOCKNUM 1024
#define MAXUSERNUM 16
#define MAXGROUPNUM 32
#define MAXUSERGROUPNUM 64

/* MODES */
#define IALLOC 0100000
#define IIMPORTANT 0200000
#define IFDIR 040000
#define ILARG 010000

#endif // STRUCTS
