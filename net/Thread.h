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

struct WorkerThread
{
	int tid;
	struct Event* notify;
	struct ConnQueue* queue;
	struct EventLoop* loop;
};


struct ConnQueue* newConnQueue();
void offer(struct ConnQueue* queue, int sockfd);
int poll(struct ConnQueue* queue);
void freeConnQueue(struct ConnQueue* queue);

#endif
