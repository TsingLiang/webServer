#ifndef BUFFEREVENT_H
#define BUFFEREVENT_H

#include <time.h>

#define DEBUG

struct BufferEvent;
struct Event;
struct EventLoop;
struct Buffer;

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

	time_t rtimeout;
	time_t wtimeout;
	
	void *arg;
};
	
struct BufferEvent* newBufferEvent(struct EventLoop* loop, int fd, 
								bufferCb readCb, bufferCb writeCb, void *arg);
void enableRead(struct BufferEvent* bevent);
void enableWrite(struct BufferEvent* bevent);
void disableWrite(struct BufferEvent* bevent);
void setReadTimeout(struct BufferEvent* bevent, time_t msec);
void setWriteTimeout(struct BufferEvent* bevent, time_t msec);
void freeBufferEvent(struct BufferEvent* bevent);

#endif
