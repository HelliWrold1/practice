//
// Created by 5099 on 2024/3/24.
//
#include "threadpool/threadpool.h"
#include "socket/socket.h"
#include "stdio.h"

int main (int argc, char *argv[])
{
    Server * server = createServer();
    server->port = 8080;
    int ret = server->run(&server->port);
    if (ret == -1)
        printf("服务器启动失败\n");
}