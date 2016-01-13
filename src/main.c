#include <stdio.h>
#include <stdlib.h>
#include "structs.h"
#include "init.h"
#include "login.h"
#include <time.h>
#include <signal.h>



/*
super_node 1024字节
inode 64字节
block 512字节
1 block = 8 inode
(2 block) (1 block) (512 inode = 64 block) (1024 block)
*/

void call_for_exit() {
    puts("");
    puts("wait for save data");
    save();
    puts("save ok");
    exit(0);
}

int main()
{


    signal(SIGINT,call_for_exit);
    signal(SIGQUIT,call_for_exit);
    signal(SIGKILL,call_for_exit);
    signal(SIGTERM,call_for_exit);

    init();
    //debug_show_dir(0);

    if(!login()) exit(0);

    shell();

    return 0;
}
