//
// Created by HelliWrold1 on 2024/3/31.
//
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include "log/log.h"

int main (int argc, char *argv[])
{
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
        tmp = redset;
        int ret = select(maxfd+1, &tmp, NULL, NULL, NULL);
        if (FD_ISSET(lfd, &tmp)) // 解除阻塞后，判断是否是监听fd的读缓冲区有数据，导致阻塞解除
        {
            int cfd = accept(lfd, NULL, NULL);
            LOG(LOG_DEBUG, "accept");
            FD_SET(cfd, &redset); // 将获取的通信fd添加到待监听的fd_set集合中
            maxfd = cfd > maxfd ? cfd: maxfd; // 使maxfd保持最大fd
        }
        // 查找fd_set中是否有通信fd解除了读缓冲阻塞
        for (int i = 0; i <=maxfd; ++i) {
            /**
             * 注意检查的fd中一定要包含maxfd
             * 如果maxfd是通信fd却没有检查
             * maxfd的读缓冲区有数据到达，可读却没有读取
             * 会导致一直解除select的阻塞
             */
            if (i!=lfd && FD_ISSET(i, &tmp))
            {
                char buf[1024];
                bzero(buf, sizeof(buf));
                int len = recv(i, buf, sizeof(buf), 0);
                if (len == -1)
                {
                    LOG(LOG_ERROR, "recv error");
                    exit(1);
                }
                else if (len == 0)
                {
                    LOG(LOG_INFO, "Client close the communication");
                    FD_CLR(i, &redset); // 如果客户端断开连接，服务端同样需要断开连接，四次挥手的最后两次
                    close(i);
                    break;
                }

                LOG(LOG_DEBUG, "read buf: %s", buf);

                for (int j = 0; j < len; ++j) {
                    buf[j] = toupper(buf[j]);
                }
                LOG(LOG_INFO, "after buf: %s", buf);

                ret = send(i, buf, strlen(buf)+1, 0);
                if (ret == -1)
                {
                    LOG(LOG_ERROR, "send error");
                    exit(1);
                }
            }
        }
    }
    close(lfd);
}

