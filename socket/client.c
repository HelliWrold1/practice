//
// Created by 5099 on 2024/3/24.
//

#include <arpa/inet.h>
#include "socket.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void client (void* arg)
{
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1)
    {
        perror("[Client] Connect");
        return;
    }
    struct sockaddr_in caddr;
    caddr.sin_family = AF_INET;
    inet_pton(AF_INET, "172.21.88.174", &caddr.sin_addr.s_addr);
    caddr.sin_port = htons(8080);
    int ret = connect(cfd, (struct sockaddr*)&caddr, sizeof (caddr));

    if (ret == -1)
    {
        perror("[Client] Connect");
        return;
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
        sleep(3);
        sendBuf(&msgInfo);
        recvBuf(&msgInfo);
    }
}