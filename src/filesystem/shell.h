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



void extract_command(char* command) {
    int len = strlen(command);
    //printf("len == %d\n", len);
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
    puts("-------");
    printf("%d\n", command_num);
    for( i = 0; i < command_num; ++i) printf("%s\n",commandbuf[i]);
    puts("-------");
    return ;
}

void shell() {
    int i, j;
    bool flag;
    char command[20][100];
    strcpy(command[0], "exit");
    strcpy(command[1], "mkdir");
    strcpy(command[2], "rmdir");
    strcpy(command[3], "mv");
    strcpy(command[4], "ls");
    strcpy(command[5], "touch");
    strcpy(command[6], "rm");
    strcpy(command[7], "cp");
    strcpy(command[8], "read");
    strcpy(command[9], "write");
    strcpy(command[10], "cd");
    strcpy(command[11], "useradd");
    strcpy(command[12], "su");
    int rst;
    while(1) {
        //printf("currnode = %d\n", curr_inode);
        printf("%s:%s%c", get_username(), get_dirname(curr_inode), (curr_user == 0 ? '#' : '$'));
        fgets(buf, 1000, stdin);
        buf[strlen(buf) - 1] = 0;
        extract_command(buf);
        if(commandbuf[0][strlen(commandbuf[0]) - 1] == '?') {
            commandbuf[0][strlen(commandbuf[0]) - 1] = 0;
            puts("show help");
            printf("%s\n",commandbuf[0]);
            continue;
        }
        rst = -1;
        for(i = 0; i < 13; ++i) {
            if(strcmp(command[i], commandbuf[0]) == 0) {
                rst = i;
                break;
            }
        }
        switch(rst) {
            case 0: {
                printf("user_num == %d\n", *user_num);
                printf("fp == %d\n",fp);
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

                if((check_path(commandbuf[1]) && check_path(commandbuf[2])) == 0) {
                    printf("mv: 路径错误\n");
                }
                command_cp(commandbuf[1], commandbuf[2]);
                break;
            }
            case 8: {
                break;
            }
            case 9: {
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
            default: {
                break;
            }




        }
    }
}





#endif // SHELL_H
