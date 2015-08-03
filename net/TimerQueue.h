#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include <stdbool.h>
#include <time.h>

#define DEBUG

struct EventLoop;
typedef void (*timeoutCallback)(struct EventLoop* loop, void* arg);

struct Timer
{
	struct Event* event;
	timeoutCallback timeCb;
	time_t start;
	time_t interval;
	int index;
};
struct TimerQueue
{
	struct Timer** timers;
	int size;
	int capacity;
};

struct TimerQueue* newTimerQueue();
void push(struct TimerQueue* queue, struct Timer* timer);
void timerQueueDel(struct TimerQueue* queue, struct Timer* timer);
void timerQueueClose(struct TimerQueue* queue);

struct Timer* top(struct TimerQueue* queue);
struct Timer* pop(struct TimerQueue* queue);
bool empty(struct TimerQueue* queue);

#endif
