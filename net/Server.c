#include "Thread.h"
#include "Event.h"
#include "Socket.h"
#include "EventLoop.h"
#include "BufferEvent.h"
#include "Server.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

static int tid = 0;

static void onNotify(struct Event* event, void* arg)
{
	assert(event != NULL && arg != NULL);

#ifdef DEBUG
	printf("on notify.\n");
#endif	
	struct WorkerThread* thread = (struct WorkerThread*)arg;
	int tid = thread->tid;
	Server* server = thread->server;
	struct ConnQueue* queue = thread->queue;
	int efd = event->fd;
	uint64_t count;	

	int nread = read(efd, &count, sizeof(count));
	assert(nread == sizeof(count));

	int sockfd = poll(queue);
	struct BufferEvent* bevent = newBufferEvent(event->loop, sockfd,
					 server->onRead, server->onWrite, bevent);
	assert(bevent != NULL);
	enableRead(bevent);
	setTimer(bevent, 6000);
	server->afterAccept(bevent, NULL);
}

static void* startThread(void* arg)
{
	struct WorkerThread* thread = (struct WorkerThread*)arg;
	struct Event* notify = thread->notify;

	eventLoopAdd(thread->loop, notify);	
	eventLoopDispatch(thread->loop);
	
	return NULL;
}

struct WorkerThread* newThread()
{
	struct WorkerThread* thread = (struct WorkerThread*)malloc
									(sizeof(struct WorkerThread));
	assert(thread != NULL);

	thread->tid = tid++;
	thread->loop = (struct EventLoop*)malloc(sizeof(struct EventLoop));
	assert(thread->loop != NULL);
	eventLoopInit(thread->loop);
	
	thread->efd = eventfd(0, 0);
	thread->notify = newEvent(thread->efd, EV_READ, onNotify, NULL,
							thread, thread->loop);
	assert(thread->notify != NULL);

	thread->queue = newConnQueue();
	assert(thread->queue != NULL); 

	pthread_t id;
	pthread_create(&id, NULL, startThread, thread);

#ifdef DEBUG
	printf("init thread: tid = %d\n", thread->tid);
#endif

	return thread;
}

void freeThread(struct WorkerThread* thread)
{
	assert(thread != NULL);

	close(thread->efd);
	eventLoopClose(thread->loop);
	free(thread->notify);
	freeConnQueue(thread->queue);
	free(thread);
}

static void onAccept(struct Event* event, void* arg)
{
	assert(event != NULL && arg != NULL);

#ifdef DEBUG
	printf("accept a connection.\n");
#endif

	Server* server = (Server*)event->arg;
	int sockfd = server->sockfd;
	assert(sockfd >= 0);
	int connfd = tcpAccept(sockfd);
	assert(connfd >= 0);

	int nextId = server->nextId % server->nthreads;
	server->nextId++;
	offer(server->threads[nextId]->queue, connfd);
	int efd = server->threads[nextId]->efd;
	uint64_t count = 1;
	int n = write(efd, &count, sizeof(count));
	assert(n == sizeof(count)); 
}

static void defaultAfterAccept(struct BufferEvent* bevent, void* arg)
{
	assert(bevent != NULL);

	int connfd = bevent->event->fd;
	char local[25];
	char peer[25];

	printf("%s -> %s.\n", getPeerAddr(connfd, peer, sizeof(peer)),
			getLocalAddr(connfd, local, sizeof(local)));
}

static void defaultOnRead(struct BufferEvent* bevent, void* arg)
{
	assert(bevent != NULL);

	bufferSwap(bevent->input, bevent->output);
	enableWrite(bevent);
}

static void defaultOnWrite(struct BufferEvent* bevent, void* arg)
{
	assert(bevent != NULL);
	
	disableWrite(bevent);
}

Server* newServer(struct EventLoop* loop, short port,
					const char* name, int len)
{
	Server* server = (Server*)malloc(sizeof(Server));
	assert(server != NULL);
	memset(server, 0, sizeof(Server));
	
	server->loop = loop;
	int n = len < sizeof(name) ? len : sizeof(name);
	strncpy(server->name, name, n);
	
	server->port = port;		

	server->afterAccept = defaultAfterAccept;
	server->onRead = defaultOnRead;
	server->onWrite = defaultOnWrite;	

	return server;
}

void setThreadNumber(Server* server, int nthreads)
{
	assert(server != NULL && nthreads > 0 && nthreads < 100);

	server->threads = (struct WorkerThread**)malloc(nthreads * 
							sizeof(struct WorkerThread*));
	assert(server->threads != NULL);

	int i;
	for(i = 0; i < nthreads; i++)
	{
		server->threads[i] = newThread();
		server->threads[i]->server = server;
	}
	server->nthreads = nthreads;
}

void setAcceptCb(Server* server, bufferCb cb)
{
	assert(server != NULL);

	server->afterAccept = cb;
}

void setReadCb(Server* server, bufferCb cb)
{
	assert(server != NULL);

	server->onRead = cb;
}

void setWriteCb(Server* server, bufferCb cb)
{
	assert(server != NULL);

	server->onWrite = cb;
}

void start(Server* server)
{
	assert(server != NULL);

	server->sockfd = tcpListen(server->port);
	assert(server->sockfd >= 0);
	getLocalAddr(server->sockfd, server->hostport, sizeof(server->hostport));

#ifdef DEBUG
	printf("start server at: %s\n", server->hostport);	
#endif	

	server->acceptor = newEvent(server->sockfd, EV_READ, onAccept, NULL,
								server, server->loop);
	assert(server->acceptor != NULL);
	eventLoopAdd(server->loop, server->acceptor);
	
	eventLoopDispatch(server->loop);
}

void stop(Server* server)
{
	assert(server != NULL);

	int i;
	for(i = 0; i < server->nthreads; i++)
	{
		freeThread(server->threads[i]);
	}
	free(server->threads);

	eventLoopDel(server->loop, server->acceptor);
	eventLoopClose(server->loop);	
}

