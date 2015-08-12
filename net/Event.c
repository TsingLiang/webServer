#include "Event.h"
#include "EventLoop.h"
#include <stdlib.h>
#include <assert.h>

struct Event* newEvent(int fd, int type, readCallback readCb, 
					writeCallback writeCb, void* arg, struct EventLoop* loop)
{
	assert(loop != NULL);

	struct Event* event = (struct Event*)malloc(sizeof(struct Event));
	assert(event != NULL);

	event->fd = fd;
	event->type = type;
	event->readCb = readCb;
	event->writeCb = writeCb;
	event->arg = arg;
	event->loop = loop;

	return event;
}

