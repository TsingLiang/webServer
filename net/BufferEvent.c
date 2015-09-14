#include "Event.h"
#include "EventLoop.h"
#include "BufferEvent.h"
#include "Buffer.h"
#include "TimerEvent.h"
#include "TimerQueue.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void onRead(struct Event* event, void* arg)
{
	struct BufferEvent* bevent = (struct BufferEvent*)arg;
	struct Buffer* input = bevent->input;
	int fd = bevent->event->fd;

	int n = bufferRead(input, fd);

#ifdef DEBUG
	printf("read %d bytes.\n", n);
#endif
	if(n > 0 && bevent->readCb != NULL)
		bevent->readCb(bevent, bevent->arg);
	else if(n <= 0 && bevent->errorCb != NULL)
		bevent->errorCb(bevent, bevent->arg);
}

static void onWrite(struct Event* event, void* arg)
{
	struct BufferEvent* bevent = (struct BufferEvent*)arg;
	struct Buffer* output = bevent->output;
	int fd = bevent->event->fd;

	int n = bufferWrite(output, fd);
	if(n > 0 && bevent->writeCb != NULL)
		bevent->writeCb(bevent, bevent->arg);
	else if(n <= 0)
		bevent->errorCb(bevent, bevent->arg);
}

static void defaultErrorCb(struct BufferEvent* bevent, void* arg)
{
	assert(bevent != NULL);

#ifdef DEBUG
	printf("error callback, fd = %d.\n", bevent->event->fd);
#endif

	freeBufferEvent(bevent);

#ifdef DEBUG
	printf("after free buffer event.\n");
#endif
}

struct BufferEvent* newBufferEvent(struct EventLoop* loop, int fd, 
								bufferCb readCb, bufferCb writeCb, void *arg)
{
	assert(loop != NULL);
	
	struct BufferEvent* bevent = (struct BufferEvent*)
									malloc(sizeof(struct BufferEvent));
	if(bevent == NULL)
		return NULL;

	bevent->loop = loop;
	bevent->readCb = readCb;
	bevent->writeCb = writeCb;
	bevent->errorCb = defaultErrorCb;
	bevent->arg = arg;
	bevent->timer = NULL;

	bevent->input = newBuffer();
	bevent->output = newBuffer();

	bevent->event = newEvent(fd, 0, onRead, onWrite, bevent, loop);
	
	return bevent;
}

void enableRead(struct BufferEvent* bevent)
{
	assert(bevent != NULL);

	struct Event* event = bevent->event;
	struct EventLoop* loop = bevent->loop;
	assert(event != NULL && loop != NULL);

	event->type |= EV_READ;	
	eventLoopAdd(loop, event);
}

void enableWrite(struct BufferEvent* bevent)
{
	assert(bevent != NULL);

	struct Event* event = bevent->event;
	struct EventLoop* loop = bevent->loop;
	assert(event != NULL && loop != NULL);

	event->type |= EV_WRITE;	
	eventLoopAdd(loop, event);
}

void disableWrite(struct BufferEvent* bevent)
{
	assert(bevent != NULL);

	struct Event* event = bevent->event;
	struct EventLoop* loop = bevent->loop;
	assert(event != NULL && loop != NULL);

	event->type &= ~EV_WRITE;
	eventLoopAdd(loop, event);
}

void setTimer(struct BufferEvent* bevent, time_t msec)
{
	assert(bevent != NULL);

	bevent->event->type |= EV_TIMER;
	bevent->timer = newTimer(bevent->event, msec);
	bevent->timer->arg = bevent;

#ifdef DEBUG
	printf("after new timer.\n");
#endif
	timerAdd(bevent->loop->tevent, bevent->timer);
}

void freeBufferEvent(struct BufferEvent* bevent)
{
	if(bevent == NULL)	
		return;

#ifdef DEBUG
	printf("free a buffer event!\n");
#endif

	if(bevent->input != NULL)
		freeBuffer(bevent->input);

	if(bevent->output != NULL)
		freeBuffer(bevent->output);

	if(bevent->timer != NULL)
	{
		if(bevent->event->type & EV_TIMER)
		{
			timerDel(bevent->loop->tevent, bevent->timer);
			bevent->event->type &= ~EV_TIMER;
		}
		free(bevent->timer);
	}

	if(bevent->event != NULL)
	{
		eventLoopDel(bevent->loop, bevent->event);
#ifdef DEBUG
		printf("before free event.\n");
#endif
		free(bevent->event);
#ifdef DEBUG
		printf("after free event.\n");
#endif

	}

#ifdef DEBUG
	printf("free buffer.\n");
#endif

	free(bevent);
 }
