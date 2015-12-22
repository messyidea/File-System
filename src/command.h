#ifndef COMMAND_H
#define COMMAND_H
#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
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
*/

char* get_name_by_uid(int id) {
    int i;
    for(i = 0; i < *user_num; ++i) {
        if(id == array_user[i]->uid) return array_user[i]->name;
    }
    return "";
}

char* get_name_by_gid(int id) {
    int i;
    for(i = 0; i < *group_num; ++i) {
        if(id == array_group[i]->gid) return array_group[i]->name;
    }
    return "";
}

char* get_mode_str(int id) {
    char str[16] = "dwrxwrxwrx";
    if(!is_dir(id)) str[0] = '-';
    int i, mod = array_inode[id]->i_mode;
    for(i = 0; i < 8; ++i) {
        if((mod & (1<<i)) != (1<<i)) {
            str[9-i] = '-';
        }
    }
    return str;
}

void command_ls(char *path) {
    int id = get_inode_from_path(path);
    if(id < 0) {
        printf("ls: 无法访问%s : 没有该路径\n", path);
        return ;
    }
    if(!is_dir(id)) {
        printf("%-20s%-10d%-15s%-15s%-15s\n", get_mode_str(id), id, get_name_by_uid(array_inode[id]->i_uid), get_name_by_gid(array_inode[id]->i_gid), path);
        return ;
    }
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
            printf("%-20s%-10d%-15s%-15s%-15s\n", get_mode_str(p_dir->inode), p_dir->inode, get_name_by_uid(array_inode[p_dir->inode]->i_uid), get_name_by_gid(array_inode[p_dir->inode]->i_gid), p_dir->name);
            //printf("%s     \n", p_dir->name);
        }
    }
    i = num - 1;
    tmpid = array_inode[id]->i_addr[i];
    get_single_block(tmpid);
    for(j = 0;j < lastnum; ++j) {
        p_dir = (struct dir*)(single_block + j * (sizeof(struct dir)));
        //printf("-----%d %d\n", p_dir->inode, id2);
        //printf("%s     \n", p_dir->name);
        printf("%-20s%-10d%-15s%-15s%-15s\n", get_mode_str(p_dir->inode), p_dir->inode, get_name_by_uid(array_inode[p_dir->inode]->i_uid), get_name_by_gid(array_inode[p_dir->inode]->i_gid), p_dir->name);
    }
    return "";
}


void command_cd(char* path) {
    int iid = get_inode_from_path(path);
    if(iid < 0) {
        printf("cd: 打开目录%s失败：文件不存在\n", path);
        return ;
    }
    if(!is_dir(iid)) {
        printf("cd: 打开目录%s失败：是文件\n", path);
        return ;
    }
    if(!have_authority(curr_user, iid, 'r')) {
        printf("cd:无法打开目录 '%s'： 没有权限\n", path);
        return -1;
    }
    curr_inode = iid;
}

int command_mkdir(char* path, bool is_important, int uid, int gid) {

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
        if(tmpid == -1 && pos < len) {
            printf("mkdir:无法创建目录%s， 没有那个文件或目录\n", path);
            return -1;
        }
    }
    if(pid < 0) return -1;
    if(find_inode_from_single_path(pid, single_pathbuf) >= 0) {
        printf("mkdir:无法创建目录 '%s'： 文件已存在\n", single_pathbuf);
        return -1;
    }
    //printf("user = %d | pid = %d | auth = %d\n", curr_user, pid, have_authority(curr_user, pid, 'w'));
    if(!have_authority(curr_user, pid, 'w')) {
        printf("mkdir:无法创建目录 '%s'： 没有权限\n", single_pathbuf);
        return -1;
    }
    iid = add_file(pid, single_pathbuf);
    set_inode_dir(iid, pid, uid, gid, is_important);

    return 1;
}

int command_touch(char* path, bool is_important, int uid, int gid) {
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
        if(tmpid == -1 && pos < len) {
            printf("touch:无法创建目录%s， 没有那个文件或目录\n", path);
            return -1;
        }
    }
    if(pid < 0) return -1;
    if(find_inode_from_single_path(pid, single_pathbuf) >= 0) {
        printf("touch:无法创建文件 '%s'： 文件已存在\n", single_pathbuf);
        return -1;
    }
    //printf("user = %d | pid = %d | auth = %d\n", curr_user, pid, have_authority(curr_user, pid, 'w'));
    if(!have_authority(curr_user, pid, 'w')) {
        printf("touch :无法创建文件 '%s'： 没有权限\n", single_pathbuf);
        return -1;
    }
    iid = add_file(pid, single_pathbuf);
    set_inode_file(iid, pid, uid, gid, is_important);
    //balloc();
    return 1;
}



int command_rmdir(char* path) {
    if(strcmp(path, ".") == 0 || strcmp(path, "./") == 0) {
        printf("rmdir: 删除'.'失败： 无效的参数\n");
        return -1;
    }
    int pid;
    int iid = get_inode_from_path(path);
    //printf("rmdir %d\n", iid);
    if(iid < 0) {
        printf("rmdir: 删除%s失败：目录不存在！\n", path);
        return -1;
    }
    //printf("isdir == %d\n", is_dir(iid));
    if(!is_dir(iid)) {
        printf("rmdir: 删除%s失败：要删除的是文件！\n", path);
        return -2;
    }
    if(!is_dir_empty(iid)) {
        printf("rmdir: 删除%s失败：目录非空\n", path);
        return -3;
    }
    //quanxian
    //root important
    if(have_authority(curr_user, iid, 'w') && is_important(iid)) {
        printf("rmdir:无法删除目录 '%s'： dir is important\n", path);
        return -5;
    }
    if(!have_authority(curr_user, iid, 'w') ) {
        printf("rmdir:无法删除目录 '%s'： 没有权限\n", path);
        return -4;
    }

    pid = find_inode_from_single_path(iid, "..");
    remove_dir_use_inode(pid, iid);
    remove_empty_dir(iid);
}

void adduser(char* username, char* password) {
    int uid, gid, iid, backup_user;
    char path_buf[100];
    uid = (*max_uid) ++;
    gid = (*max_gid) ++;
    (*user_num) ++;
    (*group_num) ++;
    (*user_group_num) ++;
    //printf("usernum == %d\n", *user_num);
    array_user[(*user_num) - 1]->uid = uid;
    strcpy(array_user[(*user_num) - 1]->passwd, password);
    strcpy(array_user[(*user_num) - 1]->name, username);

    array_group[(*group_num) - 1]->gid = gid;
    strcpy(array_group[(*group_num) - 1]->name, username);

    array_user_group[(*user_group_num) - 1]->uid = uid;
    array_user_group[(*user_group_num) - 1]->gid = gid;

    //backup_user = curr_user;
    //curr_user = uid;
    strcpy(path_buf, "/home/");
    strcat(path_buf, username);
    command_mkdir(path_buf, true, uid, gid);


    //set important


    //curr_user = backup_user;
}

bool have_blank(char* password) {
    int len = strlen(password);
    int i;
    for(i = 0; i < len ; ++i) {
        if(password[i] == ' ') return true;
    }
    return false;
}

int command_adduser(char* username) {
    char temp_path[100];
    strcpy(temp_path, "/home/");
    strcat(temp_path, username);
    int iid = get_inode_from_path(temp_path);
    if(iid >= 0) {
        printf("adduser: 增加用户%s失败： 用户目录已存在\n", username);
        return -1;
    }
    //username 必定没有空格
    int uid = get_password(username);
    char passwordbuf1[100], passwordbuf2[100];
    if(uid >= 0) {
        printf("adduser: 增加用户%s失败： 用户已存在\n", username);
        return -1;
    }
    puts("请输入密码");
    fgets(passwordbuf1, 100, stdin);
    passwordbuf1[strlen(passwordbuf1) - 1] = 0;
    if(have_blank(passwordbuf1)) {
        printf("adduser: 增加用户%s失败： 密码不能有空格\n", username);
        return -1;
    }
    puts("请再次输入密码");
    fgets(passwordbuf2, 100, stdin);
    passwordbuf2[strlen(passwordbuf2) - 1] = 0;
    if(strcmp(passwordbuf1, passwordbuf2) == 0) {
        adduser(username, passwordbuf1);
    } else {
        printf("adduser: 增加用户%s失败： 两次输入密码不一致\n", username);
    }
}

int command_su(char* username) {
    int uid = get_password(username);
    char passwordbuf1[100];
    printf("uid == %d\n", uid);
    if(uid < 0) {
        printf("su: 切换用户%s失败： 用户不存在\n", username);
        return -1;
    }
    if(curr_user == ROOTUID) {
        curr_user = uid;
        return 1;
    }
    puts("请输入密码");
    fgets(passwordbuf1, 100, stdin);
    passwordbuf1[strlen(passwordbuf1) - 1] = 0;
    if(strcmp(passwordbuf1, passwordbuf) == 0) {
        curr_user = uid;
        return 1;
    } else {
        printf("su: 切换用户%s失败： 密码错误\n", username);
        return -1;
    }

}

char* get_singlename_use_inode(int id) {
    int pid = array_inode[id]->i_pid;
    return get_singlepath_from_inode(pid, id);
}

//支持相对路径和绝对路径。目录自动判断等
int command_mv(char* path1, char* path2) {
    //printf("path1 = %s | path2 = %s\n", path1, path2);


    int idd1 = get_inode_from_path(path1);
    //printf("idd1 == %d\n", idd1);
    int idd2 = get_inode_from_path(path2);
    //printf("idd1 == %d idd2 == %d\n", idd1, idd2);
    char name[100];
    int i, j, p, k;
    if(idd1 < 0) {
        printf("mv: 错误： 不存在 %s\n", path1);
        return -2;
    }
    if(is_dir(idd1)) {
        if(idd2 >= 0 && is_dir(idd2)) {
            //把idd1 移动到文件夹 idd2 中
            strcpy(name, get_singlename_use_inode(idd1));
            if(!(have_authority(curr_user, idd1, 'r') && have_authority(curr_user, idd1, 'w') && have_authority(curr_user, idd2, 'r') && have_authority(curr_user, idd2, 'w') )) {
                printf("mv: 错误： 没有权限 %s\n");
                return -1;
            }
            if(is_important(idd1)) {
                printf("mv: 错误：%s is important\n", path1);
                return -1;
            }
            if(find_inode_from_single_path(idd2, name) > 0) {
                printf("mv: 错误：存在同名文件\n");
                return -1;
            }
            int tid = add_file(idd2, name);
            //printf("tid == %d\n", tid);
            if(tid == -1) {
                printf("mv: 错误：block空间不足 %s\n");
                return -1;
            }
            set_inode_unused(tid);
            int la, lb, yuan;
            la = (array_inode[idd2]->i_count - 1) / 16;
            lb = array_inode[idd2]->i_count - la * 16;
            lb --;
            get_single_block(array_inode[idd2]->i_addr[la]);
            p_dir = (struct dir*) (single_block + lb * (sizeof(struct dir)));
            p_dir->inode = idd1;
            strcpy(p_dir->name, name);

            array_inode[idd1]->i_pid = idd2;
            get_single_block(array_inode[idd1]->i_addr[0]);
            p_dir = (struct dir*) (single_block + 1 * (sizeof(struct dir)));
            yuan = p_dir->inode;
            p_dir->inode = idd2;

            remove_dir_use_inode(yuan, idd1);

            return 0;
        }
        //idd2 < 0
        p = 0;
        i = strlen(path2);
        i --;
        if(path2[i] == '/') i --;
        j = i;
        for(; j >= 0 && path2[j] != '/'; ) --j;
        j++;
        //j - i  就是 文件名
        p = 0;
        //printf("j - i = %d - %d\n", j , i);
        for(k = j; k <= i; ++k) name[p ++] = path2[k];
        name[p] = 0;
        //printf("debug name == %s\n", name);
        path2[j] = 0;
        //printf("path2 == %s\n", path2);
        idd2 = get_inode_from_path(path2);
        //printf("idd2 == %d\n", idd2);
        if(idd2 < 0) {
            printf("mv: 错误： 不存在 %s\n", path2);
            return -1;
        }
        if(!(have_authority(curr_user, idd1, 'r') && have_authority(curr_user, idd1, 'w') && have_authority(curr_user, idd2, 'r') && have_authority(curr_user, idd2, 'w') )) {
            printf("mv: 错误： 没有权限 %s\n");
            return -1;
        }
        if(is_important(idd1)) {
            printf("mv: 错误：%s is important\n", path1);
            return -1;
        }
        if(idd2 == array_inode[idd1]->i_pid) {
            //只需重命名
            change_dir_name(idd2, idd1, name);
            return 0;
        }
        int tid = add_file(idd2, name);
        //printf("tid == %d\n", tid);
        if(tid == -1) {
            printf("mv: 错误：block空间不足 %s\n");
            return -1;
        }
        set_inode_unused(tid);
        int la, lb, yuan;
        la = (array_inode[idd2]->i_count - 1) / 16;
        lb = array_inode[idd2]->i_count - la * 16;
        lb --;
        get_single_block(array_inode[idd2]->i_addr[la]);
        p_dir = (struct dir*) (single_block + lb * (sizeof(struct dir)));
        p_dir->inode = idd1;
        strcpy(p_dir->name, get_singlename_use_inode(idd1));

        array_inode[idd1]->i_pid = idd2;
        get_single_block(array_inode[idd1]->i_addr[0]);
        p_dir = (struct dir*) (single_block + 1 * (sizeof(struct dir)));
        yuan = p_dir->inode;
        p_dir->inode = idd2;

        remove_dir_use_inode(yuan, idd1);
        return 0;
    } else {
        //idd1 is file
        //strcpy(name, get_singlename_use_inode(idd1));
        if(idd2 >= 0 && is_dir(idd2)) {
            //把idd1 移动到文件夹 idd2 中
            strcpy(name, get_singlename_use_inode(idd1));
            int yuan = array_inode[idd1]->i_pid;
            if(!(have_authority(curr_user, idd1, 'r') && have_authority(curr_user, idd1, 'w') && have_authority(curr_user, idd2, 'r') && have_authority(curr_user, idd2, 'w') )) {
                printf("mv: 错误： 没有权限 %s\n");
                return -1;
            }
            if(is_important(idd1)) {
                printf("mv: 错误：%s is important\n", path1);
                return -1;
            }
            if(find_inode_from_single_path(idd2, name) > 0) {
                printf("mv: 错误：存在同名文件\n");
                return -1;
            }
            int tid = add_file(idd2, name);
            //printf("tid == %d\n", tid);
            if(tid == -1) {
                printf("mv: 错误：block空间不足 %s\n");
                return -1;
            }
            set_inode_unused(tid);
            int la, lb;
            la = (array_inode[idd2]->i_count - 1) / 16;
            lb = array_inode[idd2]->i_count - la * 16;
            lb --;
            get_single_block(array_inode[idd2]->i_addr[la]);
            p_dir = (struct dir*) (single_block + lb * (sizeof(struct dir)));
            p_dir->inode = idd1;
            strcpy(p_dir->name, name);

            array_inode[idd1]->i_pid = idd2;


            remove_dir_use_inode(yuan, idd1);

            return 0;
        } else {
            //idd2 is none
            int yuan = array_inode[idd1]->i_pid;
            p = 0;
            i = strlen(path2);
            i --;
            if(path2[i] == '/') i --;
            j = i;
            for(; j >= 0 && path2[j] != '/'; ) --j;
            j++;
            //j - i  就是 文件名
            p = 0;
            //printf("j - i = %d - %d\n", j , i);
            for(k = j; k <= i; ++k) name[p ++] = path2[k];
            name[p] = 0;
            //printf("debug name == %s\n", name);
            path2[j] = 0;
            //printf("path2 == %s\n", path2);
            idd2 = get_inode_from_path(path2);
            //printf("idd2 == %d\n", idd2);
            if(idd2 < 0) {
                printf("mv: 错误： 不存在 %s\n", path2);
                return -1;
            }
            if(!(have_authority(curr_user, idd1, 'r') && have_authority(curr_user, idd1, 'w') && have_authority(curr_user, idd2, 'r') && have_authority(curr_user, idd2, 'w') )) {
                printf("mv: 错误： 没有权限 %s\n");
                return -1;
            }
            if(is_important(idd1)) {
                printf("mv: 错误：%s is important\n", path1);
                return -1;
            }
            if(idd2 == array_inode[idd1]->i_pid) {
                //只需重命名
                change_dir_name(idd2, idd1, name);
                return 0;
            }
            int tid = add_file(idd2, name);
            //printf("tid == %d\n", tid);
            if(tid == -1) {
                printf("mv: 错误：block空间不足\n");
                return -1;
            }
            set_inode_unused(tid);
            int la, lb;
            la = (array_inode[idd2]->i_count - 1) / 16;
            lb = array_inode[idd2]->i_count - la * 16;
            lb --;
            get_single_block(array_inode[idd2]->i_addr[la]);
            p_dir = (struct dir*) (single_block + lb * (sizeof(struct dir)));
            p_dir->inode = idd1;
            strcpy(p_dir->name, get_singlename_use_inode(idd1));

            array_inode[idd1]->i_pid = idd2;

            remove_dir_use_inode(yuan, idd1);
            return 0;
        }

    }

    //if(!(have_authority(curr_user, idd1, 'r') && have_authority(curr_user, idd1, 'w') && have_authority(curr_user, iid2, 'r') && have_authority(curr_user, iid2, 'w')))

}

void remove_file_itself(int id) {
    int i, j, bid;
    int *num, *tid;
    int blocknum;
    blocknum = array_inode[id]->i_size;
    if(is_inode_large(id)) {
        //间接
        for(i = 0; i < blocknum; ++i) {
            bid = array_inode[id]->i_addr[i];
            get_single_block(bid);
            num = (int*)single_block;
            for(j = 0; j < *num; ++j) {
                tid = (int*)(single_block + 4*(1 + j));
                set_block_unused(*tid);
            }
            set_block_unused(bid);
        }
    } else {
        for(i = 0; i < blocknum; ++i) {
            bid = array_inode[id]->i_addr[i];
            set_block_unused(bid);
        }
    }
    set_inode_unused(id);
}


void dfs_rm(int id) {
    int i, j, k;
    int num, lastnum, bid;
    if(!is_dir(id)) {
        //remove_dir_use_inode(array_inode[id]->i_pid, id);
        remove_file_itself(id);
        return;
    } else {
        num = array_inode[id]->i_count;
        lastnum = num - (num-1) / 16 * 16;
        bid = array_inode[id]->i_addr[0];
        get_single_block(bid);
        if(num == 1) {
            for(i = 2; i < lastnum; ++i) {
                p_dir = (struct dir*)(single_block + i * (sizeof(struct dir)));
                dfs_rm(p_dir->inode);
            }
            remove_file_itself(id);
        } else {
            for(i = 2; i < 16; ++i) {
                p_dir = (struct dir*)(single_block + i * (sizeof(struct dir)));
                dfs_rm(p_dir->inode);
            }
            for(i = 1; i < num - 1; ++i) {
                bid = array_inode[id]->i_addr[i];
                get_single_block(bid);
                for(j = 0; j < 16 ; ++j) {
                    p_dir = (struct dir*)(single_block + j * (sizeof(struct dir)));
                    dfs_rm(p_dir->inode);
                }
            }
            i = num - 1;
            bid = array_inode[id]->i_addr[i];
            get_single_block(bid);
            for(i = 0; i < lastnum; ++i) {
                p_dir = (struct dir*)(single_block + i * (sizeof(struct dir)));
                dfs_rm(p_dir->inode);
            }
            remove_file_itself(id);
        }

    }
}

int command_rm(char* path, bool flag) {
    int iid;
    if(flag) {
        //printf("do rm force\n");
        iid = get_inode_from_path(path);
        if(iid < 0) {
            printf("rm: 错误：不存在 %s\n", path);
            return -1;
        }
        if(!have_authority(curr_user, iid, 'w')) {
            printf("rm: 错误: 没有权限\n");
            return -1;
        }
        if(is_important(iid)) {
            printf("rm: 错误: file is important\n");
            return -1;
        }
        if(is_dir(iid)) {
            remove_dir_use_inode(array_inode[iid]->i_pid, iid);
            remove_file_itself(iid);
        } else {
            dfs_rm(iid);
            remove_dir_use_inode(array_inode[iid]->i_pid, iid);
        }
    } else {
        //only rm files
        //printf("do rm!\n");
        iid = get_inode_from_path(path);
        if(iid < 0) {
            printf("rm: 错误：不存在 %s\n", path);
            return -1;
        }
        if(is_dir(iid)) {
            printf("rm: 错误: %s是目录, 请加上 -r\n", path);
            return -1;
        }
        if(!have_authority(curr_user, iid, 'w')) {
            printf("rm: 错误: 没有权限\n");
            return -1;
        }
        if(is_important(iid)) {
            printf("rm: 错误: file is important\n");
            return -1;
        }
        remove_dir_use_inode(array_inode[iid]->i_pid, iid);
        remove_file_itself(iid);
    }

}

char* get_pre_path(char* path) {
    //printf("path = %s\n",path);
    int i, j, len;
    char tpath[100];
    strcpy(tpath, path);
    len = strlen(tpath);
    i = len - 1;
    if(tpath[i] == '/') i --;
    for(; tpath[i] != '/' && i >= 0; i--) ;
    while(i < 0) i ++;
    tpath[i] = 0;
    if(i == 0) return "";
    return tpath;
}

char* get_back_path(char* path) {
    //printf("222 = %s\n", path);
    int i, j, len;
    char tpath[100] = "", tpath2[100] = "";
    strcpy(tpath, path);
    len = strlen(tpath);
    i = len - 1;
    if(tpath[i] == '/') i --;
    for(; tpath[i] != '/' && i >= 0; i--) ;
    //tpath[i] = 0;
    while(i < 0) i++;
    if(tpath[i] == '/') i++;
    j = 0;
    for(; i < len; ++i) {
        tpath2[j ++] = tpath[i];
    }
    tpath2[j] = 0;
    return tpath2;
}


void do_cp_file(int id1, int id2, char* name) {
    //printf("id1 quanxian222 = %d\n", array_inode[id1]->i_mode);
    //printf("name name== %s\n", name);
    int i, j, k, siz, bid, newblock;
    int *num, *bid2, *num2, *bid3;
    //char* single_block2 = (char*)(filesystem + 512*(2 + 1 + 64 + id));
    char* single_block2;
    //printf("id1 quanxian222 = %d\n", array_inode[id1]->i_mode);
    int id = add_file(id2, name);
    set_inode_file(id, id2, curr_user, curr_user, false);
    if(is_inode_large(id1)) {
        set_inode_large(id);
        siz = array_inode[id1]->i_size;
        array_inode[id]->i_size = siz;
        for(i = 0; i < siz; ++i) {
            bid = array_inode[id1]->i_addr[i];
            get_single_block(bid);
            num = (int *)single_block;
            newblock = balloc();
            array_inode[id]->i_addr[i] = newblock;
            single_block2 = (char*)(filesystem + 512*(2 + 1 + 64 + newblock));
            num2 = (int*)single_block2;
            *num2 = *num;
            for(j = 0; j < *num; ++j) {
                bid2 =  (int*)(single_block + 4 * (1 + j));
                bid3 = (int*)(single_block2 + 4 * (1 + j));
                *bid3 = balloc();
                memcpy((filesystem + 512*(2 + 1 + 64 + *bid3)), (filesystem + 512*(2 + 1 + 64 + *bid2)), 512);
            }
        }
    } else {
        //不是大文件
        siz = array_inode[id1]->i_size;
        array_inode[id]->i_size = siz;
        for(i = 0; i < siz; ++i) {
            bid = array_inode[id1]->i_addr[i];
            newblock = balloc();
            array_inode[id]->i_addr[i] = newblock;
            memcpy((filesystem + 512*(2 + 1 + 64 + newblock)), (filesystem + 512*(2 + 1 + 64 + bid)), 512);
        }
        //printf("id1 quanxian222 = %d\n", array_inode[id1]->i_mode);
    }
}


void dfs_cp(int id1, int id2) {
    int i, j, k, bid, tid, ttid;
    int num = array_inode[id1]->i_count;
    int lastnum = num - (num - 1) / 16 * 16;
    int cou = array_inode[id1]->i_size;
    for(i = 0; i < cou; ++ i) {
        bid = array_inode[id1]->i_addr[i];
        get_single_block(bid);
        k = (i == cou-1? lastnum-1:16);
        for(j = 0; j < k; ++j) {
            p_dir = (struct dir*)(single_block + j * (sizeof(struct dir)));
            if(strcmp(p_dir->name, ".") == 0 || strcmp(p_dir->name, "..") == 0) continue;
            tid = p_dir->inode;
            if(is_dir(tid)) {
                ttid = add_file(id2, p_dir->name);
                set_inode_dir(ttid, id2, curr_user, curr_user, false);
                dfs_cp(tid, ttid);
            } else {
                do_cp_file(tid, id2, p_dir->name);
            }
        }
    }
}

//cp 文件夹iid1 到文件夹 iid2中，文件名叫name
int prepare_dfs_cp(int iid1, int iid2, char* name) {
    //check size...


    int id = add_file(iid2, name);
    set_inode_dir(id, iid2, curr_user, curr_user, false);
    dfs_cp(iid1, id);
}

int command_cp(char* path1, char*path2, bool flag) {
    int i, j, k;
    int iid1, iid2;
    int ppid = get_inode_from_path(path1);
    if(ppid < 0) {
        printf("cp: 错误: %s不存在\n", path1);
        return -1;
    }
    //如果path1是文件的话，去掉-r的选项。
    if(!is_dir(iid1)) {
        flag = false;
    }
    if(flag) {
        //iid1存在且是文件
        iid1 = get_inode_from_path(path1);
        if(!have_authority(curr_user, iid1, 'r')) {
            printf("cp: 错误: %s不可读\n", path1);
            return -1;
        }
        iid2 = get_inode_from_path(path2);
        if(iid2 < 0) {
            iid2 = get_inode_from_path(get_pre_path(path2));
            if(iid2 < 0) {
                printf("cp: 错误: %s不存在\n", path2);
                return -1;
            }
            if(have_authority(curr_user, iid2, 'w')) {
                printf("cp: 错误: %s不可写\n", path2);
                return -1;
            }
            //start cp dfs
            prepare_dfs_cp(iid1, iid2, get_back_path(get_back_path(path2)));
        } else {
            if(!is_dir(iid2)) {
                printf("cp: 错误: %s已存在，是文件\n", path2);
                return -1;
            }
            if(have_authority(curr_user, iid2, 'w')) {
                printf("cp: 错误: %s不可写\n", path2);
                return -1;
            }
            //same name
            if(find_inode_from_single_path(iid2, get_back_path(path1)) > 0) {
                printf("cp: 错误: 存在同名文件\n", path2);
                return -1;
            }
            //start cp dfs
            prepare_dfs_cp(iid1, iid2, get_back_path(path1));
        }

    } else {
        iid1 = get_inode_from_path(path1);
        printf("iid1 == %d\n", iid1);
        if(iid1 < 0) {
            printf("cp: 错误: %s不存在\n", path1);
            return -1;
        }
        if(is_dir(iid1)) {
            printf("cp: 错误: %s是文件夹， use -r\n", path1);
            return -1;
        }
        if(!have_authority(curr_user, iid1, 'r')) {
            printf("cp: 错误: %s不可读\n", path1);
            return -1;
        }
        iid2 = get_inode_from_path(path2);
        //printf("iid2 == %d\n", iid2);
        //printf("path2 == = %s\n", path2);
        //printf("backpath = %s\n", get_back_path(path2));
        if(iid2 < 0) {
            //printf("prepath = %s\n", get_pre_path(path2));
            //printf("backpath = %s\n", get_back_path(path2));
            iid2 = get_inode_from_path(get_pre_path(path2));
            if(iid2 < 0) {
                printf("cp: 错误: %s不存在\n", path2);
                return -1;
            } else {
                if(!is_dir(iid2)) {
                    printf("cp: 错误: %s不存在\n", path2);
                    return -1;
                } else {
                    if(!have_authority(curr_user, iid2, 'w')) {
                        printf("cp: 错误: %s不可写\n", path2);
                        return -1;
                    }
                    if(iid2 == array_inode[iid1]->i_pid && find_inode_from_single_path(iid2, get_back_path(path2)) > 0) {
                        printf("cp: 错误: 文件已存在\n");
                        return -1;
                    }
                    do_cp_file(iid1, iid2, get_back_path(path2));
                    return 0;
                }
            }
        } else {
            if(is_dir(iid2)) {
                if(!have_authority(curr_user, iid2, 'w')) {
                    printf("cp: 错误: %s不可写\n", path2);
                    return -1;
                }
                if(find_inode_from_single_path(iid2, get_back_path(path1)) > 0) {
                    printf("cp: 错误: 存在同名文件\n", path2);
                    return -1;
                }
                do_cp_file(iid1, iid2, get_back_path(path1));
                return 0;
            } else {
                //iid2是文件，且已存在
                printf("cp: 错误: %s是已存在的文件\n", path2);
                return -1;
            }
        }
    }
}

void do_write(int pid, char *name) {
    //to do:large file
    int id = add_file(pid, name);
    if(id < 0) {
        printf("error: 没有空间，写入失败\n");
        return ;
    }
    set_inode_file(id, pid, curr_user, curr_user, false);
    int siz = strlen(inputbuf) * 2;
    int ii, jj, i, j;
    ii =  (siz + 511) / 512;
    jj = siz - siz / 512;
    array_inode[id]->i_size = ii;
    for(i = 0; i < ii; ++i) {
        array_inode[id]->i_addr[i] = balloc();
        if(array_inode[id]->i_addr[i] < 0) {
            printf("error: 没有空间，写入失败\n");
            return ;
        }
        get_single_block(array_inode[id]->i_addr[i]);
        memcpy(single_block, inputbuf + i * 512, 512);
    }
}

void do_appand(int id) {
    int siz = array_inode[id]->i_size;
    int i, j, ii, jj;
    strcpy(filesbuf, "");
    for(i = 0; i < siz; ++i) {
        int bid = array_inode[id]->i_addr[i];
        get_single_block(bid);
        strncat(filesbuf, single_block, 512/2); //字节计数
    }
    strcat(filesbuf, inputbuf);
    strcpy(inputbuf, filesbuf);
    for(i = 0; i < siz; ++i) {
        set_block_unused(array_inode[id]->i_addr[i]);
    }
    siz = strlen(inputbuf) * 2;
    ii =  (siz + 511) / 512;
    jj = siz - siz / 512;
    array_inode[id]->i_size = ii;
    for(i = 0; i < ii; ++i) {
        array_inode[id]->i_addr[i] = balloc();
        if(array_inode[id]->i_addr[i] < 0) {
            printf("error: 没有空间，写入失败\n");
            return ;
        }
        get_single_block(array_inode[id]->i_addr[i]);
        memcpy(single_block, inputbuf + i * 512, 512);
    }
}

int command_write(char* path) {
    int iid = get_inode_from_path(path);
    int pid, len, siz;
    if(iid > 0 && is_dir(iid)) {
        printf("write: 错误: %s是已存在的文件\n", path);
        return -1;
    }
    if(iid < 0) {
        pid = get_inode_from_path(get_pre_path(path));
        if(pid < 0) {
            printf("write: 错误: %s不存在\n", path);
            return -1;
        }
        if(!is_dir(pid)) {
            printf("write: 错误: 不存在该路径！\n");
            return -1;
        }
        if(!have_authority(curr_user, pid, 'w')) {
            printf("write: 错误: 没有写权限\n");
            return -1;
        }
        puts("请输入要写入的内容");
        fgets(inputbuf, 1024*256/2, stdin);
        inputbuf[strlen(inputbuf) - 1] = 0;
        do_write(pid, get_back_path(path));
        return 1;
    }
    pid = array_inode[iid]->i_pid;
    if(!have_authority(curr_user, pid, 'w')) {
        printf("write: 错误: 没有写权限\n");
        return -1;
    }
    if(is_important(iid)) {
        printf("write: 错误: %s is important， cannot change\n", path);
        return -1;
    }
    puts("请输入要添加的内容");
    fgets(inputbuf, 1024*256/2, stdin);
    inputbuf[strlen(inputbuf) - 1] = 0;
    do_appand(iid);
}

void do_cat(int id) {
    int siz = array_inode[id]->i_size;
    strcpy(filesbuf, "");
    int i, bid;
    for(i = 0; i < siz; ++i) {
        bid = array_inode[id]->i_addr[i];
        get_single_block(bid);
        strncat(filesbuf, single_block, 512/2);
    }
    printf("%s\n", filesbuf);
}

int command_cat(char* path) {
    int iid = get_inode_from_path(path);
    if(iid < 0) {
        printf("cat: 错误: %s不存在\n", path);
        return -1;
    }
    if(is_dir(iid)) {
        printf("cat: 错误: %s是文件夹\n", path);
        return -1;
    }
    if(!have_authority(curr_user, iid, 'r')) {
        printf("cat: 错误: 没有读权限\n");
        return -1;
    }
    if(is_important(iid)) {
        printf("cat: 错误: file is important, cannot show it\n");
        return -1;
    }
    do_cat(iid);
}

int command_passwd(char* name, bool need_passwd) {
    puts("do passwd");
    int uid = get_password(name);
    char pwbuf[100];
    if(uid < 0) {
        printf("passwd: 错误: 用户不存在\n");
        return -1;
    }
    if(need_passwd) {
        printf("请输入%s密码\n", name);
        fgets(pwbuf, 100, stdin);
        pwbuf[strlen(pwbuf) - 1] = 0;
        //printf("%s | %s\n",pwbuf, passwordbuf);
        if(strcmp(pwbuf, passwordbuf) != 0) {
            printf("passwd: 错误: 密码错误\n");
            return -1;
        }
    }
    puts("请输入新密码");
    fgets(pwbuf, 100, stdin);
    pwbuf[strlen(pwbuf) - 1] = 0;
    if(have_blank(pwbuf)) {
        puts("密码中不能有空格！");
        return -1;
    }
    //array_user[uid]->passwd
    strcpy(array_user[uid]->passwd, pwbuf);
}


#endif // COMMAND_H
