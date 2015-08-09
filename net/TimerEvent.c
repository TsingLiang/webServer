#include "TimerEvent.h"
#include "Event.h"
#include "EventLoop.h"
#include "TimerQueue.h"
#include <sys/timerfd.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void resetTimer(int timerfd, uint64_t expire)
{
	struct itimerspec new, old;

	memset(&new, 0, sizeof(new));
	new.it_value.tv_sec = expire / 1000;
	new.it_value.tv_nsec = expire % 1000;
	timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &new, &old);
}

static void onTimeout(struct EventLoop* loop, void* arg)
{
	assert(loop != NULL && arg != NULL);

	struct TimerEvent* tevent = (struct TimerEvent*)arg;
	struct Event* event = tevent->event;
	int timerfd = event->fd;
	uint64_t exp;
	int n = read(timerfd, &exp, sizeof(exp));
	assert(n == sizeof(exp));

	struct TimerQueue* queue = tevent->queue;
	struct Timer* latest = pop(queue);
	struct timeval tv;
	
	gettimeofday(&tv, NULL);
	uint64_t now = tv.tv_sec * 1000000 + tv.tv_usec;
	
	latest->timeCb(loop, event);

	while(!empty(queue))
	{
		latest = pop(queue);
		if(latest->start > now)
			break;

		latest->timeCb(loop, event);
	}

	if(!empty(queue))
	{
		latest = top(queue);
		resetTimer(tevent->event->fd, latest->start);	
	}
}

void timerInit(struct TimerEvent* tevent, struct EventLoop* loop)
{
	assert(tevent != NULL && loop != NULL);

#ifdef DEBUG
	printf("init timer.\n");
#endif
	
	int timerfd = timerfd_create(CLOCK_REALTIME, 0); 
	tevent->event = newEvent(timerfd, EV_READ, 
						(readCallback)onTimeout, NULL, NULL, loop);
	assert(tevent->event != NULL);
	eventLoopAdd(loop, tevent->event);

	tevent->queue = newTimerQueue();
	assert(tevent->queue != NULL);

	tevent->loop = loop;	
}

void timerAdd(struct TimerEvent* tevent, struct Timer* timer)
{
	assert(tevent != NULL && timer != NULL);

#ifdef DEBUG
	printf("add timer.\n");
#endif

	if(!empty(tevent->queue))
	{
		struct Timer* old = top(tevent->queue);
		if(old->start > timer->start)
			resetTimer(tevent->event->fd, timer->start);
	}
	else
		resetTimer(tevent->event->fd, timer->start);

	push(tevent->queue, timer);
}

void timerDel(struct TimerEvent* tevent, struct Timer* timer)
{
	assert(tevent != NULL && timer != NULL);
	struct TimerQueue* queue = tevent->queue;
	assert(!empty(queue));

#ifdef DEBUG
	printf("delete timer, index = %d\n", timer->index);
#endif

	int index = timer->index;
	timerQueueDel(queue, timer);
	if(index == 1)
	{
		struct Timer* newTimer = top(queue);
		resetTime(tevent->event->fd, newTimer->start);
	}
}

void timerClose(struct TimerEvent* tevent)
{
	assert(tevent != NULL);

	timerQueueClose(tevent->queue);
	if(tevent->event != NULL)
		free(tevent->event);

	free(tevent);
}

