#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <pthread.h>

template <class T>
class Node
{
public:
	Node(T t)
	{
		data = t;
		next = NULL;
	}

	T data;
	Node* next;
};

template <class T>
class Queue
{
public:
	Queue()
	{
		head = NULL;
		tail = NULL;
	}

	void push_back(T t)
	{
		if(head == NULL)
		{
			head = tail = new Node(t);
		}
		else
		{
			tail->next = new Node(t);
			tail = tail->next;
		}
	}

	void pop_front()
	{
		Node* tmp = head;
		head = head->next;
		delete tmp;
	}

	T front()
	{
		return head->data;
	}

	bool empty()
	{
		return head == NULL;
	}

private:
	Node *head, *tail;
}

template <class T>
class MessageQueue
{
public:
	MessageQueue()
	{
		pthread_mutex_init(&lock, NULL);
		pthread_cond_init(&cond, NULL);
	}

	~MessageQueue()
	{
		pthread_mutex_destroy(&lock);
		pthread_cond_destroy(cond);
	}

	void push(T t)
	{
		pthread_mutex_lock(&lock);
		queue.push_back(t);
		pthread_mutex_unlock(&lock);
		pthread_cond_signal(&cond);
	}

	T pop()
	{
		pthread_mutex_lock(&lock);
		while(queue.empty())
		{
			pthread_cond_wait(&cond, &lock);
		}
		T t = queue.front();
		queue.pop_front();
		pthread_mutex_unlock(&lock);

		return t;
	}
private:
	Queue<T> queue;
	pthread_mutex_t lock;
	pthread_cond_t  cond;
};

#endif
