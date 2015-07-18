#include "Socket.h"
#include <stdio.h>

int main()
{
    int acceptor = tcpListen(5000);

    while(1)
    {
        char local[25];
        char peer[25];
        int connfd = tcpAccept(acceptor);        
        
        printf("%s->%s\n", getPeerAddr(connfd, peer, sizeof(peer)),
                getLocalAddr(connfd, local, sizeof(local)));

        close(connfd); 
    }

    return 0;
}
