//
// Created by 5099 on 2024/3/24.
//

#include <arpa/inet.h>
#include "socket_multithreads/socket.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main (int argc, char *argv[])
{
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1)
    {
        perror("[Client] Connect");
        return -1;
    }
    struct sockaddr_in caddr;
    caddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &caddr.sin_addr.s_addr);
    caddr.sin_port = htons(8080);
    int ret = connect(cfd, (struct sockaddr*)&caddr, sizeof (caddr));

    if (ret == -1)
    {
        perror("[Client] Connect");
        return -1;
    }

    char ip[32];
    bzero(ip,sizeof(ip));
    // 连接建立成功，打印客户端的IP:port
    printf("[Client] IP:%s:%d\n", inet_ntop(AF_INET, &caddr.sin_addr.s_addr, ip, sizeof(ip)),
           ntohs(caddr.sin_port));
    MsgInfo msgInfo;
    msgInfo.cfd = cfd;
    sprintf(msgInfo.buff, "%s:%d\n", ip, ntohs(caddr.sin_port));
    while(1)
    {
        printf("client running...\n");
        sleep(3);
        sendBuf(&msgInfo);
        recvBuf(&msgInfo);
    }
}