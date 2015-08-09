#ifndef SERVER_H
#define SERVER_H

#define DEBUG

struct WorkerThread;

typedef struct
{
	struct Event* acceptor;
	struct EventLoop* loop;
	char   hostport[25];
	char   name[10];
	bool   start;

	struct WorkerThread* threads;
	int    nthreads;
	int    nextId;
}Server;
	

#endif
