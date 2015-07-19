#include "SignalEvent.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

const int NSIGNALS = 32;

void signalHandler(Epoll* epoll, void* arg)
{
	assert(epoll != NULL && arg != NULL);

	SignalEvent* sevent = (SignalEvent*)arg;
	size_t n = read(sevent->sigfd, &sevent->sigInfo, 
					sizeof(sevent->sigInfo));
	assert(n == sizeof(sevent->sigInfo));
	
	int signo = sevent->sigInfo.ssi_signo;
	assert(signo <= NSIGNALS);
	assert(sevent->events[signo] != NULL);

#ifdef DEBUG
	printf("handle signal event: signo = %d\n", signo);
#endif
		
	Event* event = sevent->events[signo];
	event->readCb(epoll, event);	
}

void signalInit(SignalEvent* sevent, Epoll* epoll)
{
	assert(sevent != NULL && epoll != NULL);
	
	sevent->epoll = epoll;

	sigemptyset(&sevent->mask);
	assert(sigprocmask(SIG_BLOCK, &sevent->mask, NULL) >= 0);

	sevent->sigfd = signalfd(SIG_BLOCK, &sevent->mask, 0);
	assert(sevent->sigfd >= 0);
	setNonBlock(sevent->sigfd);

	sevent->event = (Event*)malloc(sizeof(Event));
	assert(sevent->event != NULL);
	setFd(sevent->event, sevent->sigfd);
	setReadCallback(sevent->event, signalHandler);
	epollAdd(epoll, sevent->event);

	sevent->events = (Event**)malloc(NSIGNALS * sizeof(Event*));
	assert(sevent->events != NULL);
	memset(sevent->events, 0, NSIGNALS * sizeof(Event*));

#ifdef DEBUG
	printf("signal event init.\n");
#endif

}

void signalAdd(SignalEvent* sevent, Event* event)
{
	assert(sevent != NULL && event != NULL);	
#ifdef DEBUG
	printf("signal add: signum = %d\n", getFd(event));
#endif
	sigaddset(&sevent->mask, getFd(event));
	assert(sigprocmask(SIG_BLOCK, &sevent->mask, NULL) >= 0);
	assert(signalfd(sevent->sigfd, &sevent->mask, 0) >= 0);

	assert(sevent->events[getFd(event)] == NULL);
	sevent->events[getFd(event)] = event;	
}

void signalDelete(SignalEvent* sevent, Event* event)
{
	assert(sevent != NULL && event != NULL);	
#ifdef DEBUG
	printf("signal delete: signum = %d\n", getFd(event));
#endif
	sigdelset(&sevent->mask, getFd(event));
	assert(sigprocmask(SIG_BLOCK, &sevent->mask, NULL) >= 0);
	assert(signalfd(sevent->sigfd, &sevent->mask, 0) >= 0);

	assert(sevent->events[getFd(event)] != NULL);
	sevent->events[getFd(event)] = NULL;	
}

void signalClose(SignalEvent* sevent)
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

