#include "Epoll.h"
#include "Event.h"
#include <string.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

const int MAXEVENTS = 1024;

void epollInit(Epoll* epoll)
{
    epoll->epfd = epoll_create(MAXEVENTS);
    assert(epoll->epfd >= 0);
    
    epoll->epoll_events = (struct epoll_event*)malloc(
        MAXEVENTS * sizeof(struct epoll_event));
    assert(epoll->epoll_events != NULL);
    epoll->nevents = MAXEVENTS;
    
    epoll->events = (Event**)malloc(MAXEVENTS * sizeof(Event*));
    assert(epoll->events != NULL);
    memset(epoll->events, 0, MAXEVENTS * sizeof(Event*));
    epoll->nfds = MAXEVENTS;
    
    epoll->run = false;        
}

void epollAdd(Epoll* epoll, Event* event)
{
    assert(epoll != NULL);
    assert(getFd(event) >= 0);
    
    int fd = getFd(event);
    if(fd > epoll->nfds)
        return;

    struct epoll_event ev;
    ev.events = event->events;
    ev.data.ptr = event;
    epoll->events[fd] = event;
	if(epoll_ctl(epoll->epfd, EPOLL_CTL_ADD, fd, &ev) < 0)
	{
		perror("epoll add error:");
		exit(0);
	}
}

void epollDelete(Epoll* epoll, Event* event)
{
    assert(epoll != NULL);
    assert(getFd(event) >= 0);
    
    int fd = getFd(event);
    if(fd > epoll->nfds)
        return;

    struct epoll_event ev;
    ev.events = event->events;
    ev.data.ptr = event;
    epoll->events[fd] = NULL;
    epoll_ctl(epoll->epfd, fd, EPOLL_CTL_DEL, &ev);
}

void epollDispatch(Epoll* epoll, time_t msecond)
{
    assert(epoll != NULL);
 
    epoll->run = true;

#ifdef DEBUG
    printf("epoll is running.\n");
#endif

    while(epoll->run)
    {
        struct epoll_event* events = epoll->epoll_events;
        int nready = epoll_wait(epoll->epfd, 
        events, epoll->nevents, msecond);
		int i;

        for(i = 0; i < nready; i++)
        {
            Event* event = (Event*)events[i].data.ptr;
            int sockfd = getFd(event);
        
            if(events[i].events & EPOLLIN)
            {
#ifdef DEBUG
                printf("in epollDispatch: fd = %d, readCallback\n", 
                    sockfd);
#endif                
                if(event->readCb != NULL);
                    event->readCb(epoll, event);       
            }
            else if(events[i].events & EPOLLOUT)
            {
#ifdef DEBUG
                printf("in epollDispatch: fd = %d, writeCallback\n", 
                    sockfd);
#endif                
                if(event->writeCb != NULL);
                    event->writeCb(epoll, event);       
            }
        }
    }
    
}

void epollClose(Epoll* epoll)
{
    assert(epoll != NULL);

    epoll->run = false;

#ifdef DEBUG
    printf("epoll is stopped.\n");
#endif
}


