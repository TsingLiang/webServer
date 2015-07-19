#ifndef SOCKET_H
#define SOCKET_H

#define DEBUG

#include <stdbool.h>
#include <stddef.h>

struct sockaddr;
struct sockaddr_in;

int tcpListen(int port);
int tcpConnect(const char* ip, int port);
int tcpAccept(int sockfd);
void setReuseAddr(int sockfd, bool on);
void setTcpNoDelay(int sockfd, bool on);
void setKeepAlive(int sockfd, bool on);
void setNonBlock(int sockfd, bool on);

char* toIpPort(struct sockaddr_in* addr, char* buf, size_t len);
char* getLocalAddr(int sockfd, char* buf, size_t len);
char* getPeerAddr(int sockfd, char* buf, size_t len);
#endif
