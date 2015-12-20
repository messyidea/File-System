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
        for(i = 0; i < 11; ++i) {
            if(strcmp(command[i], commandbuf[0]) == 0) {
                rst = i;
                break;
            }
        }
        switch(rst) {
            case 0: {
                puts("退出");
                return ;
                break;
            }
            case 1: {
                command_mkdir(commandbuf[1]);
                break;
            }
            case 2: {
                command_rmdir(commandbuf[1]);
                break;
            }
            case 3: {
                break;
            }
            case 4: {
                command_ls(curr_inode);
                break;
            }
            case 5: {
                break;
            }
            case 6: {
                break;
            }
            case 7: {
                break;
            }
            case 8: {
                break;
            }
            case 9: {
                break;
            }
            case 10: {
                command_cd(commandbuf[1]);
                break;
            }
            default: {
                break;
            }




        }
    }
}





#endif // SHELL_H
