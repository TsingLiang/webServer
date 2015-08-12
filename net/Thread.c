#include "Thread.h"
#include "Event.h"
#include "EventLoop.h"
#include "BufferEvent.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct ConnQueue* newConnQueue()
{
	struct ConnQueue* queue = (struct ConnQueue*)malloc
									(sizeof(struct ConnQueue));
	assert(queue != NULL);

#ifdef DEBUG
	printf("new connection queue!\n");
#endif
					
	queue->head = queue->tail = NULL;
	pthread_mutex_init(&queue->lock, NULL);	

	return queue;
}

void offer(struct ConnQueue* queue, int sockfd)
{
	pthread_mutex_lock(&queue->lock);

	struct conn* conn = (struct conn*)malloc(sizeof(struct conn));
	assert(conn != NULL);
	conn->next = NULL;
	conn->sockfd = sockfd;
	if(queue->head == NULL)
	{
		queue->head = queue->tail = conn;	
	}
	else
	{
		queue->tail->next = conn;
	}
	
	pthread_mutex_unlock(&queue->lock);

#ifdef DEBUG
	printf("offer: sockfd = %d\n", sockfd);
#endif

}

int poll(struct ConnQueue* queue)
{
	pthread_mutex_lock(&queue->lock);

	struct conn* conn = queue->head;
	if(queue->head == queue->tail)
	{
		queue->head = queue->tail = NULL;
	}
	else
	{
		queue->head = conn->next;
	}
	pthread_mutex_unlock(&queue->lock);

	int sockfd = conn->sockfd;
	free(conn);

#ifdef DEBUG
	printf("pool: sockfd = %d\n", sockfd);
#endif
	
	return sockfd;
}

void freeConnQueue(struct ConnQueue* queue)
{
	while(!empty(queue))
	{
		close(poll(queue));
	}

	pthread_mutex_destroy(&queue->lock);
}
