#include "TimerQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static const int INIT_SIZE = 8; 

static void shiftUp(struct TimerQueue* queue, int i)
{
	assert(queue != NULL);

	int parent = i / 2;

	while(parent >= 1 && queue->timers[i]->start < queue->timers[parent]->start)
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

		if(queue->timers[left]->start < queue->timers[i]->start)
			min = left;

		if(right <= queue->size && 
			queue->timers[right]->start < queue->timers[min]->start)
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
	
	struct Timer** timers = (struct Timer**)realloc(queue->timers,
					(queue->capacity * 2 + 1) * sizeof(struct Timer*));
	if(timers == NULL)
		return false;

	queue->timers = timers;
	queue->capacity *= 2;

	return true;
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
	if(parent > 0 && queue->timers[parent]->start > 
						queue->timers[timer->index]->start)
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
	
	shiftDown(queue, 1);
	
	return timer;
}

bool empty(struct TimerQueue* queue)
{
	return queue->size == 0;
}


