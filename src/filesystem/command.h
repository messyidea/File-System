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
    if(is_dir(id)) str[0] = '-';
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
    printf("user = %d | pid = %d | auth = %d\n", curr_user, pid, have_authority(curr_user, pid, 'w'));
    if(!have_authority(curr_user, pid, 'w')) {
        printf("mkdir:无法创建目录 '%s'： 没有权限\n", single_pathbuf);
        return -1;
    }
    iid = add_file(pid, single_pathbuf);
    set_inode_dir(iid, pid, uid, gid, is_important);

    return 1;
}



int command_rmdir(char* path) {
    if(strcmp(path, ".") == 0 || strcmp(path, "./") == 0) {
        printf("rmdir: 删除'.'失败： 无效的参数\n");
        return -1;
    }
    int pid;
    int iid = get_inode_from_path(path);
    printf("rmdir %d\n", iid);
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
    printf("usernum == %d\n", *user_num);
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

int command_mv(char* path1, char* path2) {
    printf("path1 = %s | path2 = %s\n", path1, path2);


    int idd1 = get_inode_from_path(path1);
    printf("idd1 == %d\n", idd1);
    int idd2 = get_inode_from_path(path2);
    printf("idd1 == %d idd2 == %d\n", idd1, idd2);
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
                printf("mv: 错误：%s is important", path1);
                return -1;
            }
            int tid = add_file(idd2, name);
            printf("tid == %d\n", tid);
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
        printf("j - i = %d - %d\n", j , i);
        for(k = j; k <= i; ++k) name[p ++] = path2[k];
        name[p] = 0;
        printf("debug name == %s\n", name);
        path2[j] = 0;
        printf("path2 == %s\n", path2);
        idd2 = get_inode_from_path(path2);
        printf("idd2 == %d\n", idd2);
        if(idd2 < 0) {
            printf("mv: 错误： 不存在 %s\n", path2);
            return -1;
        }
        if(!(have_authority(curr_user, idd1, 'r') && have_authority(curr_user, idd1, 'w') && have_authority(curr_user, idd2, 'r') && have_authority(curr_user, idd2, 'w') )) {
            printf("mv: 错误： 没有权限 %s\n");
            return -1;
        }
        if(is_important(idd1)) {
            printf("mv: 错误：%s is important", path1);
            return -1;
        }
        if(idd2 == array_inode[idd1]->i_pid) {
            //只需重命名
            change_dir_name(idd2, idd1, name);
            return 0;
        }
        int tid = add_file(idd2, name);
        printf("tid == %d\n", tid);
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
                printf("mv: 错误：%s is important", path1);
                return -1;
            }
            int tid = add_file(idd2, name);
            printf("tid == %d\n", tid);
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
            printf("j - i = %d - %d\n", j , i);
            for(k = j; k <= i; ++k) name[p ++] = path2[k];
            name[p] = 0;
            printf("debug name == %s\n", name);
            path2[j] = 0;
            printf("path2 == %s\n", path2);
            idd2 = get_inode_from_path(path2);
            printf("idd2 == %d\n", idd2);
            if(idd2 < 0) {
                printf("mv: 错误： 不存在 %s\n", path2);
                return -1;
            }
            if(!(have_authority(curr_user, idd1, 'r') && have_authority(curr_user, idd1, 'w') && have_authority(curr_user, idd2, 'r') && have_authority(curr_user, idd2, 'w') )) {
                printf("mv: 错误： 没有权限 %s\n");
                return -1;
            }
            if(is_important(idd1)) {
                printf("mv: 错误：%s is important", path1);
                return -1;
            }
            if(idd2 == array_inode[idd1]->i_pid) {
                //只需重命名
                change_dir_name(idd2, idd1, name);
                return 0;
            }
            int tid = add_file(idd2, name);
            printf("tid == %d\n", tid);
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
            strcpy(p_dir->name, get_singlename_use_inode(idd1));

            array_inode[idd1]->i_pid = idd2;

            remove_dir_use_inode(yuan, idd1);
            return 0;
        }

    }

    //if(!(have_authority(curr_user, idd1, 'r') && have_authority(curr_user, idd1, 'w') && have_authority(curr_user, iid2, 'r') && have_authority(curr_user, iid2, 'w')))

}


#endif // COMMAND_H