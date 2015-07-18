#ifndef EPOLL_H
#define EPOLL_H

#include <time.h>

Event;
struct timeval;
struct epoll_event;

typedef struct _epoll
{
    int epfd;
    struct epoll_event* epoll_events;
    int nevents;

    int nfds;
    Event* events;
    bool run;
}Epoll;

void epollInit(Epoll* epoll);
void epollAdd(Epoll* epoll, Event* event);
void epollDelete(Epoll* epoll, Event* event);
void epollDispatch(Epoll* epoll, time_t msecond);
void epollClose(Epoll* epoll);

#endif
