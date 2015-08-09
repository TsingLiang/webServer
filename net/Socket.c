#include "Socket.h"
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <assert.h>

#define DEBUG

const int LISTENQ = 1024;

int tcpListen(int port)
{
    struct sockaddr_in addr;
    int acceptor = socket(AF_INET, SOCK_STREAM, 0);
    assert(acceptor > 0);
   
	setReuseAddr(acceptor, true);
 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; 
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(acceptor, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("bind error:");
        return -1;
    }
    
    listen(acceptor, LISTENQ);

#ifdef DEBUG
	char local[25];
	printf("listen in %s\n", toIpPort(&addr, local, sizeof(local)));
#endif
	
    return acceptor;
}

int tcpConnect(const char* ip, int port)
{
    struct sockaddr_in addr;
    int connector = socket(AF_INET, SOCK_STREAM, 0);
    assert(connector > 0);
   
    if(ip == NULL)
        return -1;
 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; 
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(connect(connector, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("connect error:");
        return -1;
    }
    
    return connector;
}

int tcpAccept(int sockfd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    return accept(sockfd, (struct sockaddr*)&addr, &len);
}

void setReuseAddr(int sockfd, bool on)
{
    int value = on ? 1 : 0;
    
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
                    &value, sizeof(value));
}

void setTcpNoDelay(int sockfd, bool on)
{
    int value = on ? 1 : 0;
    
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, 
                    &value, sizeof(value));
}

void setKeepAlive(int sockfd, bool on)
{
    int value = on ? 1 : 0;
    
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, 
                    &value, sizeof(value));
}

void setNonBlock(int sockfd, bool on)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flags);    
}

char* toIpPort(struct sockaddr_in* addr, char* buf, size_t len)
{
    char ip[20];
    int  port;

    if(addr == NULL || buf == NULL)
        return NULL;

    inet_ntop(AF_INET, &addr->sin_addr, ip, (socklen_t)sizeof(struct sockaddr));
    port = ntohs(addr->sin_port);

    snprintf(buf, len, "%s:%u", ip, port);

    return buf;
}

char* getLocalAddr(int sockfd, char* buf, size_t len)
{
    struct sockaddr_in addr;
    socklen_t socklen = sizeof(addr);
    
    getsockname(sockfd, (struct sockaddr*)&addr, &socklen);
    
    return toIpPort(&addr, buf, len);
}

char* getPeerAddr(int sockfd, char* buf, size_t len)
{
    struct sockaddr_in addr;
    socklen_t socklen = sizeof(addr);
    
    getpeername(sockfd, (struct sockaddr*)&addr, &socklen);
    
    return toIpPort(&addr, buf, len);
}
