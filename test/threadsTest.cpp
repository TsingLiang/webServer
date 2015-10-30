#include "MessageQueue.h"

#include <stdio.h>
#include <vector>

using namespace std;

class Input
{
public:
	Input(int n1 = 0, int n2 = 0)
	{
		this->n1 = n1;
		this->n2 = n2;
	}

	int n1;
	int n2;
};

class Output
{
public:
	Output(Input input, int n3 = 0)
	{
		this->n1 = input.n1;
		this->n2 = input.n2;
		this->n3 = n3;
	}

	Output(int n1 = 0, int n2 = 0, int n3 = 0)
	{
		this->n1 = n1;
		this->n2 = n2;
		this->n3 = n3;
	}

	int n1;
	int n2;
	int n3;
};

typedef void* (*func)(void*);

class MyThreadPool
{
public:
	MyThreadPool(int nthreads = 4)
	{
		this->nthreads = nthreads;
		next = 0;

		for(int i = 0; i < this->nthreads; i++)
		{
			queues.push_back(MessageQueue<Input>());
		}
	}

	void run()
	{
		pthread_t tid;

		void* mainfunc(void*);

		pthread_create(&tid, NULL, mainfunc, NULL);
	}

	void push(Input input)
	{
		in.push(input);
	}

	Output pop()
	{
		return out.pop();
	}

	MessageQueue<Input> in;
	MessageQueue<Output> out;
	vector< MessageQueue<Input> > queues;
	int nthreads;
	int next;
};

MyThreadPool pool;

void* mainfunc(void* arg)
{
	for(int i = 0; i < pool.nthreads; i++)
	{
		void* worker(void*);

		pthread_t tid;
		pthread_create(&tid, NULL, worker, &pool.queues[i]);
	}
	while(1)
	{
		Input input = pool.in.pop();
		pool.next = pool.next % pool.nthreads;
		pool.queues[pool.next].push(input);
		pool.next++;
	}

	return NULL;
}

void* worker(void* arg)
{
	MessageQueue<Input>* queue = (MessageQueue<Input>*)arg; 

	while(1)
	{
		Input input = queue->pop();
		Output output(input, 0);
		output.n3 = input.n1 + input.n2;
		pool.out.push(output);
	}

	return NULL;
}

int main()
{
	pool.run();

	for(int i = 0; i < 2000; i++)
	{
		pool.push(Input(i, i + 1));
	}

	for(int i = 0; i < 2000; i++)
	{
		Output output = pool.pop();
		if(output.n1 + output.n2 != output.n3)
			printf("%d + %d = %d\n", output.n1, output.n2, output.n3);
	}

	printf("compute over!\n");

	return 0;
}
