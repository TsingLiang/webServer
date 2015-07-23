#ifndef EVENT_H
#define EVENT_H

struct EventLoop;

typedef void (*readCallback)(struct EventLoop* loop, void* arg);
typedef void (*writeCallback)(struct EventLoop* loop, void *arg);

struct Event
{
    int fd;
    int type;
    readCallback readCb;
    writeCallback writeCb;
	struct EventLoop* loop;
};

struct Event* newEvent(int fd, int type, readCallback readCb, 
						writeCallback writeCb, struct EventLoop* loop);
#endif
