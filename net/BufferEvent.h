#ifndef BUFFEREVENT_H
#define BUFFEREVENT_H

#include <time.h>

#define DEBUG

struct BufferEvent;
struct Event;
struct EventLoop;
struct Buffer;
struct Timer;

typedef void (*bufferCb)(struct BufferEvent* bevent, void* arg);

struct BufferEvent
{
	struct EventLoop* loop;
	struct Event* event;
	
	struct Buffer* input;
	struct Buffer* output;

	bufferCb readCb;
	bufferCb writeCb;
	bufferCb errorCb;

	struct Timer* timer;

	void *arg;
};
	
struct BufferEvent* newBufferEvent(struct EventLoop* loop, int fd, 
								bufferCb readCb, bufferCb writeCb, void *arg);
void enableRead(struct BufferEvent* bevent);
void enableWrite(struct BufferEvent* bevent);
void disableWrite(struct BufferEvent* bevent);
void setTimer(struct BufferEvent* bevent, time_t msec);
void freeBufferEvent(struct BufferEvent* bevent);

#endif
