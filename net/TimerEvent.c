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

static void resetTimer(int timerfd, struct timeval expire)
{
	struct itimerspec new, old;

#ifdef DEBUG
	printf("reset timer!\n");
#endif

	memset(&new, 0, sizeof(new));
	new.it_value.tv_sec = expire.tv_sec;
	new.it_value.tv_nsec = expire.tv_usec * 1000;
	timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &new, &old);
}

static void onTimeout(struct EventLoop* loop, void* arg)
{
	assert(loop != NULL && arg != NULL);

#ifdef DEBUG
	printf("process timeout event!\n");
#endif

	struct TimerEvent* tevent = (struct TimerEvent*)arg;
	struct Event* event = tevent->event;
	int timerfd = event->fd;
	uint64_t exp;
	int n = read(timerfd, &exp, sizeof(exp));
	assert(n == sizeof(exp));

	struct TimerQueue* queue = tevent->queue;
	struct Timer* latest = pop(queue);
	struct timeval now;
	
	gettimeofday(&now, NULL);
	
	latest->event->type &= ~EV_TIMER;
	latest->timeCb(loop, latest->arg);

	while(!empty(queue))
	{
		latest = top(queue);
		if(timeCmp(latest->expire, now) > 0)
			break;

		latest = pop(queue);
		latest->event->type &= ~EV_TIMER;
		latest->timeCb(loop, latest->arg);
	}

	if(!empty(queue))
	{
		latest = top(queue);
		resetTimer(tevent->event->fd, latest->expire);	
	}
	else
	{
		struct timeval expire;
		gettimeofday(&expire, NULL);
		expire.tv_sec += 31536000; //one year
		resetTimer(tevent->event->fd, expire);
	}
}

static void defaultTimeout(struct EventLoop* loop, void* arg)
{
	assert(loop != NULL);
	
#ifdef DEBUG
	printf("event timeout!\n");
#endif

	struct BufferEvent* bevent = (struct BufferEvent*)arg;
	freeBufferEvent(bevent);	
}

void timerInit(struct TimerEvent* tevent, struct EventLoop* loop)
{
	assert(tevent != NULL && loop != NULL);

#ifdef DEBUG
	printf("init timer.\n");
#endif
	
	int timerfd = timerfd_create(CLOCK_REALTIME, 0); 
	tevent->event = newEvent(timerfd, EV_READ, 
						(readCallback)onTimeout, NULL, tevent, loop);
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
		if(timeCmp(old->expire, timer->expire) > 0)
			resetTimer(tevent->event->fd, timer->expire);
	}
	else
		resetTimer(tevent->event->fd, timer->expire);

	push(tevent->queue, timer);
}

void timerDel(struct TimerEvent* tevent, struct Timer* timer)
{
	assert(tevent != NULL && timer != NULL);
	struct TimerQueue* queue = tevent->queue;
#ifdef DEBUG
	printf("delete timer, index = %d\n", timer->index);
	printf("queue: size = %d, capacity = %d\n", queue->size, queue->capacity);
#endif
	assert(empty(queue) == false);

	int index = timer->index;
	timerQueueDel(queue, timer);
	if(!empty(queue) && index == 1)
	{
		struct Timer* newTimer = top(queue);
		resetTimer(tevent->event->fd, newTimer->expire);
	}
	else if(empty(queue))
	{
		struct timeval expire;
		gettimeofday(&expire, NULL);
		expire.tv_sec += 31536000; //one year
		resetTimer(tevent->event->fd, expire);
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

struct Timer* newTimer(struct Event* event, time_t timeout)
{
	assert(event != NULL);

	struct Timer* timer = (struct Timer*)malloc(sizeof(struct Timer));
	if(timer == NULL)
		return NULL;

	timer->event = event;
	timer->timeCb = defaultTimeout;
	
	gettimeofday(&timer->expire, NULL);
	timer->expire.tv_sec += timeout / 1000;
	timer->expire.tv_usec += (timeout % 1000) * 1000;
	timer->index = -1;

#ifdef DEBUG
	printf("expire: %lu: %lu \n", timer->expire.tv_sec, timer->expire.tv_usec);
#endif

	return timer;
}
