#ifndef EVENT_H
#define EVENT_H

#include <time.h>

struct Event;

typedef void (*readCallback)(struct Event* event, void* arg);
typedef void (*writeCallback)(struct Event* event, void *arg);

struct Event
{
    int fd;
    int type;
    readCallback readCb;
    writeCallback writeCb;
	void* arg;

	struct EventLoop* loop;
};

struct Event* newEvent(int fd, int type, readCallback readCb, 
					writeCallback writeCb, void* arg, struct EventLoop* loop);

#endif
