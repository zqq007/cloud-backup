#pragma once

#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

void daemon()
{
    // 忽略信号
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    // 防止自己是进程组组长
    if (fork() > 0)
        exit(0);

    //新建会话，让自己成为会话中第一个进程
    pid_t id = setsid();
    if(id == -1) exit(-1);

    //处理后续对于标准输入、标准输出、标准错误的问题
    int fd = open("/dev/null", O_RDWR);
    if(fd < 0) exit(-2);
    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);
    close(fd);
}