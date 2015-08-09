#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#define DEBUG

struct Epoll;
struct SignalEvent;
struct Event;
struct TimerEvent;

#define	EV_READ  	1
#define	EV_WRITE  	2
#define	EV_SIGNAL  	4
#define	EV_TIMER  	8

struct EventLoop
{
	struct Epoll* epoll;
	struct SignalEvent* sevent;
	struct TimerEvent* tevent;
};

void eventLoopInit(struct EventLoop* loop);
void eventLoopAdd(struct EventLoop* loop,  struct Event* event);
void eventLoopDel(struct EventLoop* loop,  struct Event* event);
void eventLoopDispatch(struct EventLoop* loop);
void eventLoopClose(struct EventLoop* loop);

#endif
