#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>

#define DEBUG

struct EventLoop;
struct Event;
typedef void (*timeoutCallback)(struct EventLoop* loop, void* arg);

struct Timer
{
	struct Event* event;
	timeoutCallback timeCb;
	void* arg;
	struct timeval expire;
	int index;
};

struct TimerQueue
{
	struct Timer** timers;
	int size;
	int capacity;
};
int timeCmp(struct timeval left, struct timeval right);

struct TimerQueue* newTimerQueue();
void push(struct TimerQueue* queue, struct Timer* timer);
void timerQueueDel(struct TimerQueue* queue, struct Timer* timer);
void timerQueueClose(struct TimerQueue* queue);

struct Timer* top(struct TimerQueue* queue);
struct Timer* pop(struct TimerQueue* queue);
bool empty(struct TimerQueue* queue);

#endif
