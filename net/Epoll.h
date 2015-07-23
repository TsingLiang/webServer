#ifndef EPOLL_H
#define EPOLL_H

#define DEBUG

#include <time.h>
#include <stdbool.h>

struct timeval;
struct epoll_event;
struct Event;

struct Epoll
{
    int epfd;
    struct epoll_event* epoll_events;
    int nevents;

    int nfds;
	struct Event** events;
    bool run;
};

void epollInit(struct Epoll* epoll);
void epollAdd(struct Epoll* epoll, struct Event* event);
void epollDelete(struct Epoll* epoll, struct Event* event);
void epollDispatch(struct Epoll* epoll, time_t msecond);
void epollClose(struct Epoll* epoll);

#endif
