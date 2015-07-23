#include "SignalEvent.h"
#include "Event.h"
#include "EventLoop.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

const int NSIGNALS = 32;

void signalHandler(struct EventLoop* loop, void* arg)
{
	assert(loop != NULL && arg != NULL);

	struct SignalEvent* sevent = loop->sevent;
	size_t n = read(sevent->sigfd, &sevent->sigInfo, 
					sizeof(sevent->sigInfo));
	assert(n == sizeof(sevent->sigInfo));
	
	int signo = sevent->sigInfo.ssi_signo;
	assert(signo <= NSIGNALS);
	assert(sevent->events[signo] != NULL);

#ifdef DEBUG
	printf("handle signal event: signo = %d\n", signo);
#endif
		
	struct Event* event = sevent->events[signo];
	event->readCb(loop, event);	
}

void signalInit(struct SignalEvent* sevent, struct EventLoop* loop)
{
	assert(sevent != NULL && loop != NULL);
	
	sevent->loop = loop;
	struct Epoll* epoll = loop->epoll;

	sigemptyset(&sevent->mask);
	assert(sigprocmask(SIG_BLOCK, &sevent->mask, NULL) >= 0);

	sevent->sigfd = signalfd(-1, &sevent->mask, 0);
	assert(sevent->sigfd >= 0);
	setNonBlock(sevent->sigfd);

	sevent->event = newEvent(sevent->sigfd, EV_READ, 
					signalHandler, NULL, loop);
	assert(sevent->event != NULL);
	epollAdd(epoll, sevent->event);

	sevent->events = (struct Event**)malloc(NSIGNALS * sizeof(struct Event*));
	assert(sevent->events != NULL);
	memset(sevent->events, 0, NSIGNALS * sizeof(struct Event*));

#ifdef DEBUG
	printf("signal event init.\n");
#endif

}

void signalAdd(struct SignalEvent* sevent, struct Event* event)
{
	assert(sevent != NULL && event != NULL);	
#ifdef DEBUG
	printf("signal add: signum = %d\n", event->fd);
#endif
	sigaddset(&sevent->mask, event->fd);
	assert(sigprocmask(SIG_BLOCK, &sevent->mask, NULL) >= 0);
	assert(signalfd(sevent->sigfd, &sevent->mask, 0) >= 0);

	assert(sevent->events[event->fd] == NULL);
	sevent->events[event->fd] = event;	
}

void signalDel(struct SignalEvent* sevent, struct Event* event)
{
	assert(sevent != NULL && event != NULL);	
#ifdef DEBUG
	printf("signal delete: signum = %d\n", event->fd);
#endif
	sigdelset(&sevent->mask, event->fd);
	assert(sigprocmask(SIG_BLOCK, &sevent->mask, NULL) >= 0);
	assert(signalfd(sevent->sigfd, &sevent->mask, 0) >= 0);

	assert(sevent->events[event->fd] != NULL);
	sevent->events[event->fd] = NULL;	
}

void signalClose(struct SignalEvent* sevent)
{
	assert(sevent != NULL);
#ifdef DEBUG
	printf("signal close.\n");
#endif
	
	if(sevent->event != NULL)
		free(sevent->event);

	int i;
	for(i = 0; i < NSIGNALS ; i++)
	{
		if(sevent->events[i] != NULL)
			free(sevent->events[i]);
	}

	if(sevent->events != NULL)
		free(sevent->events);
}

