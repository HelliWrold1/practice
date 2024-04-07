//
// Created by HelliWrold1 on 2024/4/2.
//
#include "socket.h"

int main (int argc, char *argv[])
{
    Server * server = createServer();
    server->port = 8080;
    int ret = server->run(&server->port);
    if (ret == -1)
        printf("服务器启动失败\n");
}