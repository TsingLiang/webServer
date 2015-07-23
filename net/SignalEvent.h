#ifndef SIGNALEVENT_H
#define SIGNALEVENT_H

#define DEBUG

#include <sys/signalfd.h>
#include <signal.h>

struct Event;
struct EventLoop;

struct SignalEvent
{
	int sigfd;
	sigset_t mask;
	struct signalfd_siginfo sigInfo;
	struct Event* event;
	struct Event** events;
	struct EventLoop* loop;
};

void signalInit(struct SignalEvent* sevent, struct EventLoop* loop);
void signalAdd(struct SignalEvent* sevent, struct Event* event);
void signalDel(struct SignalEvent* sevent, struct Event* event);
void signalClose(struct SignalEvent* sevent);

#endif
