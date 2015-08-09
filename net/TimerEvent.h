#ifndef TIMEREVENT_H
#define TIMEREVENT_H

#include <time.h>

#define DEBUG

struct EventLoop;
struct Timer;
struct TimerQueue;
struct Event;

struct TimerEvent
{
	struct Event* event;
	struct TimerQueue* queue;
	struct EventLoop* loop;
};


void timerInit(struct TimerEvent* tevent, struct EventLoop* loop);
void timerAdd(struct TimerEvent* tevent, struct Timer* timer);
void timerDel(struct TimerEvent* tevent, struct Timer* timer);
void timerClose(struct TimerEvent* tevent);
	
struct Timer* newTimer(struct Event* event, time_t timeout);
#endif
