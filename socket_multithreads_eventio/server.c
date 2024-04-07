//
// Created by HelliWrold1 on 2024/3/31.
//
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <sys/select.h>
#include "log/log.h"
#include <pthread.h>

static pthread_mutex_t sel_mutex;

typedef struct fdinfo
{
    int fd;
    int *maxfd;
    fd_set *rdset;
}FD_INFO;

void *acceptConn(void *arg)
{
    FD_INFO *info = (FD_INFO*)arg;
    if (!info)
        return NULL;
    int cfd = accept(info->fd, NULL, NULL);
    pthread_mutex_lock(&sel_mutex);
    FD_SET(cfd, info->rdset);
    *info->maxfd = cfd > *info->maxfd ? cfd: *info->maxfd;
    pthread_mutex_unlock(&sel_mutex);
    if (info)
        free(info);
    info = NULL;
    return NULL;
}

void * communication(void *arg)
{
    char buf[1024];
    FD_INFO *info = (FD_INFO*)arg;
    if (!info)
        return NULL;
    bzero(buf, sizeof(buf));
    LOG(LOG_INFO, "tid: %#lx", pthread_self());
    int len = recv(info->fd, buf, sizeof(buf), 0);
    if (len == -1)
    {
        LOG(LOG_ERROR, "recv error");
        if (info)
            free(info);
        info = NULL;
        return NULL;
    }
    else if (len == 0)
    {
        LOG(LOG_INFO, "Client close the communication");
        pthread_mutex_lock(&sel_mutex);
        FD_CLR(info->fd, info->rdset); // 如果客户端断开连接，服务端同样需要断开连接，四次挥手的最后两次
        pthread_mutex_unlock(&sel_mutex);
        close(info->fd);
        if (info)
            free(info);
        info = NULL;
        return NULL;
    }

    LOG(LOG_DEBUG, "read buf: %s", buf);

    for (int j = 0; j < len; ++j) {
        buf[j] = toupper(buf[j]);
    }
    LOG(LOG_INFO, "after buf: %s", buf);

    int ret = send(info->fd, buf, strlen(buf)+1, 0);
    if (ret == -1)
    {
        LOG(LOG_ERROR, "send error");
    }
    if (info)
        free(info);
    info = NULL;
    return NULL;
}

int main (int argc, char *argv[])
{
    pthread_mutex_init(&sel_mutex, NULL);
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        LOG(LOG_ERROR, "create lfd error");
        exit(1);
    }

    struct sockaddr_in sockaddrIn;
    sockaddrIn.sin_port = htons(8080);
    sockaddrIn.sin_family = AF_INET;
    sockaddrIn.sin_addr.s_addr = INADDR_ANY;

    int ret = bind(lfd, (struct sockaddr*)&sockaddrIn, sizeof(sockaddrIn));
    if (ret == -1)
    {
        LOG(LOG_ERROR, "bind error");
        exit(1);
    }

    ret = listen(lfd, 1024);
    if (ret == -1)
    {
        LOG(LOG_ERROR, "listen error");
        exit(1);
    }

    fd_set redset;
    fd_set tmp;
    FD_ZERO(&redset);
    FD_SET(lfd, &redset);

    int maxfd = lfd;

    while (1)
    {
        static int info = 1;
        if (info == 0)
            LOG_DEBUG("Unblock select");
        if (info == 1)
        {
            info = 0;
            LOG(LOG_INFO, "server is running...");
        }
        pthread_mutex_lock(&sel_mutex);
        tmp = redset;
        pthread_mutex_unlock(&sel_mutex);
        int ret = select(maxfd+1, &tmp, NULL, NULL, NULL);
        if (FD_ISSET(lfd, &tmp)) // 解除阻塞后，判断是否是监听fd的读缓冲区有数据，导致阻塞解除
        {
            LOG(LOG_INFO, "accept");
            // 创建子线程
            pthread_t tid;
            FD_INFO *info = (FD_INFO*)malloc(sizeof(FD_INFO));
            info->fd = lfd;
            info->maxfd = &maxfd;
            info->rdset = &redset;
            pthread_create(&tid, NULL, acceptConn, info);
            pthread_detach(tid);
        }
        // 查找fd_set中是否有通信fd解除了读缓冲阻塞
        for (int i = 0; i <=maxfd; ++i)
        {
            if (i != lfd && FD_ISSET(i, &tmp))
            {
                pthread_t tid;
//                pthread_attr_t attr;
//                struct sched_param param;
//                pthread_attr_init(&attr);
//
//                pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
//                param.sched_priority = 0;
//                pthread_attr_setschedparam(&attr, &param);

                // 创建线程
                FD_INFO *info = (FD_INFO*)malloc(sizeof(FD_INFO));
                info->fd = i;
                info->rdset = &redset;
//                pthread_create(&tid, &attr, communication, info);
                pthread_create(&tid, NULL, communication, info);
                pthread_detach(tid);
            }
        }
    }
    close(lfd);
    pthread_mutex_destroy(&sel_mutex);
}

