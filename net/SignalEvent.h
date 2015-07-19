#ifndef SIGNALEVENT_H
#define SIGNALEVENT_H

#define DEBUG

#include <sys/signalfd.h>
#include <signal.h>
#include "Event.h"
#include "Epoll.h"

typedef struct SignalEvent
{
	int sigfd;
	sigset_t mask;
	struct signalfd_siginfo sigInfo;
	Event* event;
	Event** events;
	Epoll* epoll;
}SignalEvent;

void signalInit(SignalEvent* sevent, Epoll* epoll);
void signalAdd(SignalEvent* sevent, Event* event);
void signalDelete(SignalEvent* sevent, Event* event);
void signalClose(SignalEvent* sevent);

#endif
