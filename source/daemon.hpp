#pragma once

#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

void daemon()
{
    // �����ź�
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    // ��ֹ�Լ��ǽ������鳤
    if (fork() > 0)
        exit(0);

    //�½��Ự�����Լ���Ϊ�Ự�е�һ������
    pid_t id = setsid();
    if(id == -1) exit(-1);

    //����������ڱ�׼���롢��׼�������׼���������
    int fd = open("/dev/null", O_RDWR);
    if(fd < 0) exit(-2);
    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);
    close(fd);
}