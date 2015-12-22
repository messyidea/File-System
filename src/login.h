#ifndef LOGIN_H
#define LOGIN_H
#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int get_password(char *username) {
    int i;
    for(i = 0; i < *user_num; ++i) {
        //printf("%s | %s\n", array_user[i]->name, username);
        if(strcmp(array_user[i]->name, username) == 0) {
            strcpy(passwordbuf, array_user[i]->passwd);
            return i;
        }
    }
    return -1;
}

bool login() {
    int i, j, len, pos = 0, uid, p;
    while(1) {
INPUT_USER_NAME:
        puts("please input your username");
        fgets(buf, 1000, stdin);
        len = strlen(buf);
        for(i = 0; i < len; ++i) {
            if(buf[i] == ' ') {
                puts("unvailid username");
                pos ++;
                if(pos == 3) {
                    puts("tried max times, exit");
                    return false;
                }
                goto INPUT_USER_NAME;
            }
        }
        // the last char is '\n'
        buf[strlen(buf) - 1] = 0;
        uid = get_password(buf);
        if(uid == -1) {
            puts("no such user!");
            pos ++;
            if(pos == 3) {
                    puts("tried max times, exit");
                    return false;
            }
            goto INPUT_USER_NAME;
        }
        strcpy(usernamebuf, buf);
        puts("please input password");
        fgets(buf, 1000, stdin);
        buf[strlen(buf) - 1] = 0;
        if(strcmp(buf, passwordbuf) == 0) {
            curr_user = uid;
            curr_inode = 0;
            if(uid == 0) {
                curr_inode = get_inode_from_path("/root");
            } else {
                strcpy(buf, "/home/");
                strcat(buf, usernamebuf);
                curr_inode = get_inode_from_path(buf);
            }
            puts("login success!");
            return true;
        } else {
            puts("password wrong.");
            pos ++;
            if(pos == 3) {
                puts("tried max times, exit");
                return false;
            }
            goto INPUT_USER_NAME;
        }
        //printf("%d %s\n",strlen(buf), buf);
        return ;

    }
    return ;
}




#endif
