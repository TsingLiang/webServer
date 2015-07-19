#include "EventLoop.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


static char table[3][10] = {"EV_IO", 
							"EV_SIGNAL",
							"EV_TIMER"};

void EventLoopInit(EventLoop* loop)
{
	assert(loop != NULL);

#ifdef DEBUG
	printf("event loop init.\n");
#endif

	loop->epoll = (Epoll*)malloc(sizeof(Epoll));
	assert(loop->epoll != NULL);
	EpollInit(loop->epoll);
	
	loop->sevent = (SignalEvent*)malloc(sizeof(SignalEvent));
	assert(loop->sevent != NULL);
	signalInit(loop->sevent, loop->epoll);
}

void EventLoopAdd(EventLoop* loop, enum EventType type, Event* event)
{
	assert(loop != NULL && event != NULL);

#ifdef DEBUG
	printf("event loop add: type = %s\n", table[type]);
#endif	
	
	switch(type)
	{
		case EV_IO:
			epollAdd(loop->epoll, event);
			break;
		
		case EV_SIGNAL:
			signalAdd(loop->sevent, event);
			break;

		case EV_TIMER:
			break;

		default:
			printf("unkown event\n");
			break;
	}
}

void EventLoopDel(EventLoop* loop, enum EventType type, Event* event)
{
	assert(loop != NULL && event != NULL);

#ifdef DEBUG
	printf("event loop  delete: type = %s\n", table[type]);
#endif	
	
	switch(type)
	{
		case EV_IO:
			epollDelete(loop->epoll, event);
			break;
		
		case EV_SIGNAL:
			signalDel(loop->sevent, event);
			break;

		case EV_TIMER:
			break;

		default:
			printf("unkown event\n");
			break;
	}	
}

void EventLoopDispatch(EventLoop* loop)
{
	assert(loop != NULL);
	
	epollDispatch(loop->epoll, -1);
}

void EventLoopClose(EventLoop* loop)
{
	assert(loop != NULL);

	epollClose(loop->epoll);
}


