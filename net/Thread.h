#ifndef THREAD
#define THREAD

#include <pthread.h>

#define DEBUG

struct conn
{
	int sockfd;
	struct conn* next;	
};

struct ConnQueue
{
	struct conn* head;
	struct conn* tail;

	pthread_mutex_t lock;
};

struct Event;
struct EventLoop;
struct Server;

struct WorkerThread
{
	int tid;
	int efd;
	struct Event* notify;
	struct ConnQueue* queue;
	struct EventLoop* loop;

	struct Server* server;
};


struct ConnQueue* newConnQueue();
void offer(struct ConnQueue* queue, int sockfd);
int poll(struct ConnQueue* queue);
void freeConnQueue(struct ConnQueue* queue);

struct WorkerThread* newThread();
void freeThread(struct WorkerThread* thread);

#endif
