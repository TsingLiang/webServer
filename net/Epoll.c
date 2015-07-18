#include "Epoll.h"
#include <string.h>
#include <sys/epoll.h>
#include <sys/time.h>

const int MAXEVENTS = 1024;

void epollInit(Epoll* epoll)
{
    epoll->epfd = epoll_create(1);
    assert(epoll->epfd >= 0);
    
    epoll->epoll_events = (struct epoll_event*)malloc(
        MAXEVENTS * sizeof(struct epoll_event));
    assert(epoll->epoll_events != NULL);
    epoll->nevents = MAXEVENTS;
    
    epoll->events = (Event*)malloc(MAXEVENTS * sizeof(Event));
    assert(epoll->events != NULL);
    epoll->nfds = MAXEVENTS;
    
    epoll->run = false;        
}

void epollAdd(Epoll* epoll, Event* event)
{
        
}

void epollDelete(Epoll* epoll, Event* event);
void epollDispatch(Epoll* epoll, time_t msecond);
void epollClose(Epoll* epoll);


