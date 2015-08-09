#include "TimerQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static const int INIT_SIZE = 8; 

static void shiftUp(struct TimerQueue* queue, int i)
{
	assert(queue != NULL);

	int parent = i / 2;

	while(parent >= 1 && 
		timeCmp(queue->timers[i]->expire, queue->timers[parent]->expire) < 0)
	{
		struct Timer* timer = queue->timers[i];
		queue->timers[i] = queue->timers[parent];
		queue->timers[parent] = timer;

		i = parent;
		parent = i / 2;
	}
}

static void shiftDown(struct TimerQueue* queue, int i)
{
	int left;
	int right;
	int min;

	assert(queue != NULL);

	while(i * 2 <= queue->size)
	{
		min = i;
		left = i * 2;
		right = i * 2 + 1;

		if(timeCmp(queue->timers[left]->expire, queue->timers[i]->expire) < 0)
			min = left;

		if(right <= queue->size && 
		timeCmp(queue->timers[right]->expire, queue->timers[min]->expire) < 0)
			min = right;

		if(min == i)
			break;

		struct Timer* timer = queue->timers[i];
		queue->timers[i] = queue->timers[min];
		queue->timers[min] = timer;
		
		i = min;
	}
}

static bool expand(struct TimerQueue* queue)
{
	assert(queue != NULL);

#ifdef DEBUG
	printf("expand timer queue!\n");
#endif
	
	struct Timer** timers = (struct Timer**)realloc(queue->timers,
					(queue->capacity * 2 + 1) * sizeof(struct Timer*));
	if(timers == NULL)
		return false;

	queue->timers = timers;
	queue->capacity *= 2;

	return true;
}
  
int timeCmp(struct timeval left, struct timeval right)
{
	if(left.tv_sec > right.tv_sec || 
		(left.tv_sec == right.tv_sec && left.tv_usec > right.tv_usec))
		return 1;

	if(left.tv_sec == right.tv_sec && left.tv_usec == right.tv_usec)
		return 0;

	return -1;	
}

struct TimerQueue* newTimerQueue()
{
	struct TimerQueue* queue = (struct TimerQueue*)malloc
								(sizeof(struct TimerQueue));
	assert(queue != NULL);

	queue->timers = (struct Timer**)malloc
						((INIT_SIZE + 1) * sizeof(struct Timer*));
	assert(queue->timers != NULL);
	queue->size = 0;
	queue->capacity = INIT_SIZE;
	
#ifdef DEBUG
	printf("new timer queue!\n");
#endif

	return queue;	
}

void timerQueueDel(struct TimerQueue* queue, struct Timer* timer)
{
	assert(queue != NULL && timer != NULL);

	queue->timers[timer->index] = queue->timers[queue->size];
	queue->size--;

	int parent = timer->index / 2;
	if(parent > 0 && timeCmp(queue->timers[parent]->expire,
						queue->timers[timer->index]->expire))
		shiftUp(queue, timer->index);
	else 
		shiftDown(queue, timer->index);
	
	timer->index = -1;
}

void push(struct TimerQueue* queue, struct Timer* timer)
{
	assert(queue != NULL && timer != NULL);

	if(queue->size == queue->capacity)
	{
		if(expand(queue) == false)
			return;
	}

	queue->timers[++queue->size] = timer;
	timer->index = queue->size;
	shiftUp(queue, queue->size);
#ifdef DEBUG
	printf("push timer: size = %d\n", queue->size);
#endif

}

struct Timer* top(struct TimerQueue* queue)
{
	assert(queue != NULL);

	return queue->timers[1];
}

struct Timer* pop(struct TimerQueue* queue)
{
	assert(queue != NULL);

	struct Timer* timer = queue->timers[1];
	queue->timers[1] = queue->timers[queue->size];
	queue->size--;

#ifdef DEBUG
	printf("pop timer: size = %d\n", queue->size);
#endif	
	shiftDown(queue, 1);
	
	return timer;
}

bool empty(struct TimerQueue* queue)
{
	return queue->size == 0;
}

void timerQueueClose(struct TimerQueue* queue)
{
	assert(queue != NULL);

	int i;
	for(i = 0; i <= queue->size; i++)
	{
		free(queue->timers[i]);
	}

	free(queue);
}
