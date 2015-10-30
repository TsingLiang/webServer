#include "MessageQueue.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

static const int THREADS = 4;

void* run(void* arg)
{
	MessageQueue<int>* mq = (MessageQueue<int>*)arg;

	while(1)
	{
		int message = mq->pop();

		printf("message = %d\n", message);
	}

	return NULL;
}

int main()
{	
	MessageQueue<int>* mq = new MessageQueue<int>();

	for(int i = 0; i < THREADS; i++)
	{
		pthread_t tid;
		pthread_create(&tid, NULL, run, mq);
	}

	for(int i = 0; i < 10; i++)
		mq->push(i);

	sleep(1);

	return 0;
}
