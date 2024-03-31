//
// Created by 5099 on 2024/3/24.
//

#ifndef PRACTICE_SOCKET_H
#define PRACTICE_SOCKET_H
#include "threadpool/threadpool.h"
#include <pthread.h>

typedef struct server Server;

typedef struct msginfo MsgInfo;

typedef struct server
{
    int listen_fd;
    unsigned short port;
    int (*run)(int *port);
    ThreadPool *pool;
    pthread_mutex_t server_mutex;
}Server;

typedef struct msginfo
{
    int cfd;
    char buff[1024];
}MsgInfo;

Server* createServer();
int runServer(int *port);
void serverListen(void *arg);
void Communicate(void* arg);
int sendBuf(MsgInfo *msgInfo);
int recvBuf(MsgInfo *msgInfo);

#endif