#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include "BufferEvent.h"

#define DEBUG

struct WorkerThread;
struct Event;
struct EventLoop;


struct Server
{
	int    sockfd;
	struct Event* acceptor;
	struct EventLoop* loop;
	int	   port;
	char   hostport[25];
	char   name[10];
	bool   start;

	struct WorkerThread** threads;
	int    nthreads;
	int    nextId;

	bufferCb afterAccept;
	bufferCb onRead;
	bufferCb onWrite;
};

typedef struct Server Server;

Server* newServer(struct EventLoop* loop, short port,
					const char* name, int len);
void setThreadNumber(Server* server, int nthreads);
void setAcceptCb(Server* server, bufferCb cb);
void setReadCb(Server* server, bufferCb cb);
void setWriteCb(Server* server, bufferCb cb);
void start(Server* server);
void restart(Server* server);
void stop(Server* server);

#endif
