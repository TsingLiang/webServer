#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "Epoll.h"
#include "SignalEvent.h"
#include "Event.h"

#define DEBUG

typedef struct EventLoop
{
	Epoll* epoll;
	SignalEvent* sevent;
}EventLoop;

enum EventType
{
	EV_IO,
	EV_SIGNAL,
	EV_TIMER
};

void EventLoopInit(EventLoop* loop);
void EventLoopAdd(EventLoop* loop, enum EventType type, Event* event);
void EventLoopDel(EventLoop* loop, enum EventType type, Event* event);
void EventLoopDispatch(EventLoop* loop);
void EventLoopClose(EventLoop* loop);

#endif
