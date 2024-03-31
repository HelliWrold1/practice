#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <arpa/inet.h>
#include <string.h>
#include "log/log.h"

#define PORT (8080)

int main(int argc, char const *argv[])
{
    // 创建通信套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        return -1;
    }
    // 连接服务器
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr.s_addr);
//	saddr.sin_addr.s_addr = ; // 0.0.0.0
    int ret = connect(fd, (struct sockaddr*)&saddr, sizeof(saddr));
    if (ret == -1)
    {
        perror("connect error");
        return -1;
    }
    LOG(LOG_INFO, "Connection is built");
    while (1)
    {
        char buff[1024];
        bzero(buff, sizeof(buff));
        gets(buff);
        int len = send(fd, buff, strlen(buff) + 1, 0);
        LOG(LOG_DEBUG, "send len :%d", len);
        LOG_INFO("send: %s", buff);
        len = recv(fd, buff, sizeof(buff), 0);
        if (len > 0)
        {
            buff[len] = "\0";
            LOG_INFO("recv: %s", buff);
            // send(fd, buff, len, 0);
        }
        else if (len == 0)
        {
            LOG(LOG_INFO, "server closed communication\n");
            break;
        }
        else
        {
            LOG(LOG_ERROR, "recv error");
            break;
        }
    }

    close(fd);
    return 0;
}
