#include "Epoll.h"
#include "Socket.h"
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>

void onAccept(Epoll* epoll, void* arg)
{ 
	Event* event = (Event*)arg;
	char local[25];
    char peer[25];
	int acceptor = getFd(event);
    int connfd = tcpAccept(acceptor);        
        
    printf("%s->%s\n", getPeerAddr(connfd, peer, sizeof(peer)),
            getLocalAddr(connfd, local, sizeof(local)));

    close(connfd);	
}

int main()
{
	Epoll* epoll = (Epoll*)malloc(sizeof(Epoll));
	Event* event = (Event*)malloc(sizeof(Event));
    int acceptor = tcpListen(5000);
	setFd(event, acceptor);
	setReadCallback(event, onAccept);
	setEvents(event, EPOLLIN);

	epollInit(epoll);
	epollAdd(epoll, event);

	epollDispatch(epoll, -1);

	return 0;
}
