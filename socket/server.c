//
// Created by 5099 on 2024/3/24.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "threadpool/threadpool.h"
#include "socket/socket.h"

static int quit = 0;

void sigintHandler()
{
    quit = 1;
}

void serverListen(void *arg)
{
    Server* server = (Server*)arg;
    if (server == NULL)
    {
        printf("[Server] Server pointer is null\n");
        return;
    }
    pthread_mutex_lock(&server->server_mutex);
    int cfd = -1;
    while(1)
    {
        // 阻塞并等待客户端连接
        struct sockaddr_in caddr;
        int addrlen = sizeof(caddr);
        threadPoolPollAddTask(server->pool, "Client Communication task", client, NULL);
        if (server->listen_fd)
            cfd = accept(server->listen_fd, (struct sockaddr*)&caddr, &addrlen); // 用于通信的fd
        else
        {
            printf("[Server] Server.listen_fd is invalid\n");
            break;
        }
        if (cfd == -1)
        {
            perror("[Server] accept");
        }
        char ip[32];
        // 连接建立成功，打印客户端的IP:port
        printf("[Server: %p ] IP:%s:%d\n", server, inet_ntop(AF_INET, &caddr.sin_addr.s_addr, ip, sizeof(ip)),
               ntohs(caddr.sin_port));
        threadPoolPollAddTask(server->pool, "Server Communication task", Communicate, (void*)cfd);
    }
}

void getIPInfo(int fd, char *ip, int *port)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    getpeername(fd, (struct sockaddr *)&client_addr, &client_len);

    // 将网络字节序的IP地址转换为字符串形式
    inet_ntop(AF_INET, &(client_addr.sin_addr), ip, INET_ADDRSTRLEN);
    *port = client_addr.sin_port;
}

void Communicate(void* arg)
{
    MsgInfo msgInfo;
    bzero(&msgInfo, sizeof(MsgInfo));
    msgInfo.cfd = (int)arg;
    recvBuf(&msgInfo);
    bzero(&msgInfo.buff, sizeof(msgInfo.buff));
    sprintf(msgInfo.buff, "%s %d\n", "Feedback for", msgInfo.cfd);
    sendBuf(&msgInfo);
}

int sendBuf(MsgInfo *msgInfo)
{
    int len =  send(msgInfo->cfd, msgInfo->buff, sizeof(msgInfo->buff), 0);
    if (len < 0)
        printf("[Send:%4d] Send error\n", msgInfo->cfd);
    else
    {
        char ip[INET6_ADDRSTRLEN];
        int port;
        getIPInfo(msgInfo->cfd, ip, &port);
        msgInfo->buff[sizeof(msgInfo->buff) - 1] = 0;
        if (len == 0)
            printf("[Send:%4d] [%s:%d] close connection\n", msgInfo->cfd, ip, port);
        else
            printf("[Send:%4d] [%s:%d]: %s\n", msgInfo->cfd, ip, port, msgInfo->buff);
    }
    return len;
}

int recvBuf(MsgInfo *msgInfo)
{
    bzero(msgInfo->buff, sizeof(msgInfo->buff));
    int len =  recv(msgInfo->cfd, msgInfo->buff, sizeof(msgInfo->buff), 0);
    if (len < 0)
        printf("[Recv:%4d] Recv Error\n", msgInfo->cfd);
    else
    {
        char ip[INET_ADDRSTRLEN];
        int port;
        getIPInfo(msgInfo->cfd, ip, &port);
        if (len == 0)
            printf("[Recv:%4d] [%s:%d] close connection\n", msgInfo->cfd, ip, port);
        else
            printf("[Recv:%4d] [%s:%d]: %s\n", msgInfo->cfd, ip, port, msgInfo->buff);
    }
    return len;
}

int runServer(int *port)
{
    // 创建监听套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("[Server] socket");
        return -1;
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(*port);
    saddr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
    int ret = bind(fd, (struct sockaddr*)&saddr, sizeof(saddr));
    if (ret == -1)
    {
        perror("[Server] bind error");
        return -1;
    }

    ret = listen(fd, 128);
    if (ret == -1)
    {
        perror("[Server] listen error");
        return -1;
    }

    Server * server = (Server *)((unsigned short int*)port - (size_t)&((Server*)0)->port);

    if(server)
        server->listen_fd = fd;
    else
        return -1;

    char ip[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET, &saddr.sin_addr.s_addr, ip, sizeof(ip));
    printf("[Server] Listening on %s:%d\n", ip, *port);
    // 创建一个监听线程
    ThreadPool* socket_pool = threadPoolCreate(10, 128, 5, 128);
    if (socket_pool)
        server->pool = socket_pool;
    else
    {
        close(fd);
        free(server);
        return -1;
    }
    threadPoolPollAddTask(socket_pool, "listen", serverListen, server);
    sleep(10); // 线程休眠让出CPU，让listen线程抢到锁，下一句就可以阻塞了
    pthread_mutex_lock(&server->server_mutex);
    close(fd);
    return 0;
}

Server *createServer()
{
    Server* server = (Server*)malloc(sizeof (Server));
    if (server)
    {
        server->run = runServer;
        pthread_mutex_init(&server->server_mutex, NULL);
        return server;
    }
    else
        return NULL;
}

