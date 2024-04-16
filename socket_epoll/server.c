//
// Created by HelliWrold1 on 2024/4/17.
//

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "log/log.h"

int main (int argc, char *argv[]) {
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        LOG(LOG_ERROR, "create lfd error");
        exit(1);
    }

    struct sockaddr_in sockaddrIn;
    sockaddrIn.sin_port = htons(8080);
    sockaddrIn.sin_family = AF_INET;
    sockaddrIn.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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

   int epfd =  epoll_create(100);
    if (epfd == -1)
    {
        perror("Epoll create");
        exit(0);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;    // 检测lfd读读缓冲区是否有数据
    ev.data.fd = lfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev); // 每次添加都是将epoll_event拷贝到红黑树中
    if(ret == -1)
    {
        perror("epoll_ctl");
        exit(0);
    }

    struct epoll_event evs[1024];
    int size = sizeof(evs) / sizeof(struct epoll_event);
    // 持续检测
    while(1)
    {
        int num = epoll_wait(epfd, evs, size, -1); // 阻塞，有监听的事件发生时解除阻塞
        // 遍历事件， 根据文件描述符种类做出相应处理
        for (int i = 0; i < num; i++)
        {
            int fd = evs[i].data.fd;
            if (fd == lfd)
            {
                LOG_INFO("accept");
                int cfd = accept(fd, NULL, NULL);
                ev.events = EPOLLIN;
                ev.data.fd = cfd;
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
                if(ret == -1)
                {
                    perror("epoll_ctl-accept");
                    exit(0);
                }
            }
            else
            {
                LOG_INFO("receiving");
                char buf[1024];
                bzero(buf, sizeof(buf));
                int len = recv(fd, buf, sizeof(buf), 0);
                if (len == -1)
                {
                    LOG(LOG_ERROR, "recv error");
                    exit(1);
                }
                else if (len == 0)
                {
                    LOG(LOG_INFO, "Client close the communication");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(i);
                    break;
                }

                LOG(LOG_INFO, "read buf: %s", buf);

                for (int j = 0; j < len; ++j) {
                    buf[j] = toupper(buf[j]);
                }
                LOG(LOG_INFO, "after buf: %s", buf);

                ret = send(fd, buf, strlen(buf)+1, 0);
                if (ret == -1)
                {
                    LOG(LOG_ERROR, "send error");
                    exit(1);
                }
            }
        }
    }
    close(lfd);
    return 0;
}