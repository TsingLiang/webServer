#include "Event.h"
#include "EventLoop.h"
#include "BufferEvent.h"
#include "Buffer.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void onRead(struct EventLoop* loop, void* arg)
{
	struct BufferEvent* bevent = (struct BufferEvent*)arg;
	struct Buffer* input = bevent->input;
	int fd = bevent->event->fd;

	int n = bufferRead(input, fd);
	if(n > 0 && bevent->readCb != NULL)
		bevent->readCb(bevent, bevent->arg);
	else if(n <= 0)
		bevent->errorCb(bevent, bevent->arg);
}

static void onWrite(struct EventLoop* loop, void* arg)
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

	freeBufferEvent(bevent);
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
	bevent->rtimeout = 0;
	bevent->wtimeout = 0;

	bevent->input = newBuffer();
	bevent->output = newBuffer();

	bevent->event = newEvent(fd, 0, onRead, onWrite, loop);
	
	return bevent;
}

void enableRead(struct BufferEvent* bevent)
{
	assert(bevent != NULL);

	struct Event* event = bevent->event;
	struct EventLoop* loop = bevent->loop;
	assert(event != NULL && loop != NULL);

	event->type |= EV_READ;	
	EventLoopAdd(loop, event);
}

void enableWrite(struct BufferEvent* bevent)
{
	assert(bevent != NULL);

	struct Event* event = bevent->event;
	struct EventLoop* loop = bevent->loop;
	assert(event != NULL && loop != NULL);

	event->type |= EV_WRITE;	
	EventLoopAdd(loop, event);
}

void disableWrite(struct BufferEvent* bevent)
{
	assert(bevent != NULL);

	struct Event* event = bevent->event;
	struct EventLoop* loop = bevent->loop;
	assert(event != NULL && loop != NULL);

	EventLoopDel(loop, event);
	event->type &= ~EV_WRITE;	
}

void setReadTimeout(struct BufferEvent* bevent, time_t msec)
{
}
void setWriteTimeout(struct BufferEvent* bevent, time_t msec)
{
}
void freeBufferEvent(struct BufferEvent* bevent)
{
	if(bevent == NULL)	
		return;
	
	if(bevent->event != NULL)
	{
		eventLoopDel(bevent->loop, bevent->event);
		free(bevent->event);
	}

	if(bevent->input != NULL)
		freeBuffer(bevent->input);

	if(bevent->output != NULL)
		freeBuffer(bevent->output);

	free(bevent);
 }
