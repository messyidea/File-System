#ifndef SHELL_H
#define SHELL_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "structs.h"
#include "common.h"
#include "command.h"


// 解开命令，command_num表示命令的字符串数目，每个字符串存放在commandbuf中
void extract_command(char* command) {
    int len = strlen(command);
    int i, j, p;
    command_num = 0;
    for(i = 0; i < len; ++i) {
        if(command[i] == ' ') {
            continue;
        }
        for(j = 0; command[i] != ' ' && i < len; ) {
            commandbuf[command_num][j++] = command[i++];
        }
        commandbuf[command_num][j] = 0;
        command_num ++;
    }
    /*
    puts("-------");
    printf("%d\n", command_num);
    for( i = 0; i < command_num; ++i) printf("%s\n",commandbuf[i]);
    puts("-------");
    */
    return ;
}

void shell() {
    printf("start shell\n");
    int i, j;
    bool flag;
    char command[15][100];
    strcpy(command[0], "exit");
    strcpy(command[1], "mkdir");
    strcpy(command[2], "rmdir");
    strcpy(command[3], "mv");
    strcpy(command[4], "ls");
    strcpy(command[5], "touch");
    strcpy(command[6], "rm");
    strcpy(command[7], "cp");
    strcpy(command[8], "cat");
    strcpy(command[9], "write");
    strcpy(command[10], "cd");
    strcpy(command[11], "useradd");
    strcpy(command[12], "su");
    strcpy(command[13], "passwd");
    strcpy(command[14], "du");
    int rst;
    inputbuf = (char*)malloc(1024*256);
    filesbuf = (char*)malloc(1024*256);
    while(1) {
        printf("%s:%s%c", get_username(), get_dirname(curr_inode), (curr_user == 0 ? '#' : '$'));
        fgets(buf, 1000, stdin);
        buf[strlen(buf) - 1] = 0;
        extract_command(buf);
        if(commandbuf[0][strlen(commandbuf[0]) - 1] == '?') {
            commandbuf[0][strlen(commandbuf[0]) - 1] = 0;
            puts("show help");
            rst = -1;
            for(i = 0; i < 15; ++i) {
                if(strcmp(command[i], commandbuf[0]) == 0) {
                    rst = i;
                    break;
                }
            }
            switch(rst) {
                case 0: {
                    puts("exit: 退出");
                    break;
                }
                case 1: {
                    puts("mkdir: 创建文件： mkdir name1 name2 ...");
                    break;
                }
                case 2: {
                    puts("rmdir: 删除文件: rmdir path1 path2 ...");
                    break;
                }
                case 3: {
                    puts("mv: 移动文件： mv path1 path2");
                    break;
                }
                case 4: {
                    puts("ls: 列出文件: ls path     or    ls");
                    break;
                }
                case 5: {
                    puts("touch: 创建空文件： touch name1 name2 ...");
                    break;
                }
                case 6: {
                    puts("rm: 删除文件: rm name1 name2 ...\n 如果要删除文件夹，请在最后加-r");
                    break;
                }
                case 7: {
                    puts("cp: 复制文件： cp file1 path \n 如果要移动文件夹， 请在最后加上-r");
                    break;
                }
                case 8: {
                    puts("cat： 显示文件： cat file");
                    break;
                }
                case 9: {
                    puts("write: 写入文件： write file");
                    break;
                }
                case 10: {
                    puts("cd: 切换目录： cd path");
                    break;
                }
                case 11: {
                    puts("useradd： 增加用户： useradd username");
                    break;
                }
                case 12: {
                    puts("su： 切换用户： su username");
                    break;
                }
                case 13: {
                    puts("passwd： 修改用户密码： passwd username");
                    break;
                }
                case 13: {
                    puts("du： 显示文件或者文件夹体积：du path");
                    break;
                }
                default: {
                    puts("unknow command");
                    break;
                }
            }
            continue;
        }
        rst = -1;
        for(i = 0; i < 15; ++i) {
            if(strcmp(command[i], commandbuf[0]) == 0) {
                rst = i;
                break;
            }
        }
        switch(rst) {
            case 0: {
                save();
                puts("退出");

                return ;
                break;
            }
            case 1: {
                if(command_num == 1) {
                    printf("mkdir: 缺少参数\n");
                    break;
                }
                for(i = 1; i < command_num; ++i) {
                    if( check_path(commandbuf[i]) )
                        command_mkdir(commandbuf[i], false, curr_user, curr_user);
                    else
                        printf("mkdir: 创建%s失败：路径不符合规范\n", commandbuf[i]);
                }
                //command_mkdir(commandbuf[1]);
                break;
            }
            case 2: {
                if(command_num == 1) {
                    printf("rmdir: 缺少参数\n");
                    break;
                }
                for(i = 1; i < command_num; ++i) {
                    if( check_path(commandbuf[i]) )
                        command_rmdir(commandbuf[i]);
                    else
                        printf("rmdir: 删除%s失败：路径不符合规范\n", commandbuf[i]);
                }
                break;
            }
            case 3: {
                if(command_num <= 2 || command_num > 3) {
                    printf("mv: 参数错误\n");
                    break;
                }
                if((check_path(commandbuf[1]) && check_path(commandbuf[2])) == 0) {
                    printf("mv: 路径错误\n");
                }
                command_mv(commandbuf[1], commandbuf[2]);
                break;
            }
            case 4: {
                if(command_num > 2) {
                    printf("ls: 参数错误\n");
                    break;
                }
                if(command_num == 1) {
                    command_ls(get_dirname(curr_inode));
                    break;
                } else {
                    command_ls(commandbuf[1]);
                    break;
                }
                break;
            }
            case 5: {

                if(command_num == 1) {
                    printf("touch: 缺少参数\n");
                    break;
                }
                for(i = 1; i < command_num; ++i) {
                    if( check_path(commandbuf[i]) )
                        command_touch(commandbuf[i], false, curr_user, curr_user);
                    else
                        printf("touch: 创建%s失败：路径不符合规范\n", commandbuf[i]);
                }

                break;
            }
            case 6: {
                if(command_num == 1) {
                    printf("rm: 缺少参数\n");
                    break;
                }
                flag = false;
                if(strcmp(commandbuf[command_num - 1], "-r") == 0) {
                    flag = true;
                    command_num --;
                }
                for(i = 1; i < command_num; ++i) {
                    if( check_path(commandbuf[i]) )
                        command_rm(commandbuf[i], flag);
                    else
                        printf("rm: rm%s失败：路径不符合规范\n", commandbuf[i]);
                }
                break;
            }
            case 7: {
                if(command_num <= 2 || command_num > 4) {
                    printf("cp: 参数错误\n");
                    break;
                }
                if(command_num == 4 && strcmp(commandbuf[3], "-r") != 0) {
                    printf("cp: 参数错误\n");
                    break;
                }
                flag = false;
                if(command_num == 4 && strcmp(commandbuf[3], "-r") == 0) flag = true;

                if((check_path(commandbuf[1]) && check_path(commandbuf[2])) == 0) {
                    printf("cp: 路径错误\n");
                    break;
                }
                command_cp(commandbuf[1], commandbuf[2], flag);
                break;
            }
            case 8: {
                if(command_num == 1 || command_num > 2) {
                    printf("cat: 参数错误\n");
                    break;
                }
                if(check_path(commandbuf[1])) {
                    command_cat(commandbuf[1]);
                } else {
                    printf("cat: 路径错误\n");
                    break;
                }
                break;
            }
            case 9: {
                if(command_num == 1 || command_num > 2) {
                    printf("write: 参数错误\n");
                    break;
                }
                if(check_path(commandbuf[1])) {
                    command_write(commandbuf[1]);
                } else {
                    printf("write: 路径错误\n");
                    break;
                }
                break;
            }
            case 10: {
                if(command_num == 1 || command_num > 2) {
                    printf("cd: 参数错误\n");
                    break;
                }
                if(!check_path(commandbuf[1])) {
                    printf("cd: 打开%s失败： 路径有误\n", commandbuf[1]);
                    break;
                }
                command_cd(commandbuf[1]);
                break;
            }
            case 11: {
                if(curr_user != ROOTUID) {
                    printf("useradd：Permission denied.\n");
                    break;
                }
                if(command_num == 1 || command_num > 2) {
                    printf("useradd：参数错误\n");
                    break;
                }
                command_adduser(commandbuf[1]);
                break;
            }
            case 12: {
                if(command_num == 1 || command_num > 2) {
                    printf("su：参数错误\n");
                    break;
                }

                command_su(commandbuf[1]);
                break;
            }
            case 13: {
                if(command_num > 2) {
                    printf("su：参数错误\n");
                    break;
                }
                if(command_num == 1) {
                    command_passwd(get_username(), false);
                    break;
                }
                if(curr_user == 0) {
                    command_passwd(commandbuf[1], false);
                } else {
                    command_passwd(commandbuf[1], true);
                }
                break;
            }
            case 13: {
                if(command_num != 2) {
                    printf("du：参数错误\n");
                    break;
                }
                if(!check_path(commandbuf[1])) {
                    printf("du: 失败： 路径有误\n", commandbuf[1]);
                    break;
                }
                command_du(commandbuf[1]);
                break;
            }
            default: {
                puts("unknow command");
                break;
            }




        }
    }
}





#endif // SHELL_H
