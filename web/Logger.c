#include "Logger.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>


static struct Logger* logger = NULL;
static const int LOG_BUF_SIZE = 64;
static const int DEFAULT_THRESH = 1000000;

static struct LogQueue* newLogQueue()
{
	struct LogQueue* queue = (struct LogQueue*)malloc(sizeof(struct LogQueue));
	assert(queue != NULL);
	
	queue->head = NULL;
	queue->tail = NULL;
	
	pthread_mutex_init(&queue->lock, NULL);
	pthread_cond_init(&queue->cond, NULL);

	return queue;	
} 

static void pushLog(struct LogQueue* queue, char* buf, int len, int level)
{
	assert(queue != NULL && buf != NULL);

	struct LogNode* node = (struct LogNode*)malloc(sizeof(struct LogNode));
	assert(node != NULL);

	node->buf = buf;
	node->len = len;
	node->level = level;
	node->next = NULL;

	pthread_mutex_lock(&queue->lock);

	if(queue->head == NULL)
	{
		queue->head = queue->tail = node;
	}
	else
	{
		queue->tail->next  = node;
		queue->tail = node;
	}

	pthread_cond_signal(&queue->cond);
	pthread_mutex_unlock(&queue->lock);
}

static struct LogNode* popLog(struct LogQueue* queue)
{
	assert(queue != NULL);

	pthread_mutex_lock(&queue->lock);

	while(queue->head == NULL)
	{
		pthread_cond_wait(&queue->cond, &queue->lock);
	}

	struct LogNode* node = queue->head;
	queue->head = node->next;
	if(queue->head == NULL)
	{
		queue->tail = NULL;
	}

	pthread_mutex_unlock(&queue->lock);

	return node;
}

static void* logThread(void* arg)
{
	assert(logger != NULL);

	while(logger->run)
	{
		struct LogNode* node = popLog(logger->queue);
		assert(node != NULL);

		if(node->level >= logger->level)
		{
			int n = write(logger->logFile, node->buf, node->len);
			assert(n == node->len);
			
			if(++logger->logCount >= logger->thresh)
			{
				close(logger->logFile);
				
				char name[70];
				
				strncpy(name, logger->filePath, 70);
				snprintf(name + strlen(name), sizeof(name) - strlen(name),
							"_%d", ++logger->fileCount);
				rename(logger->filePath, name);

				logger->logFile = open(logger->filePath, 
										O_CREAT | O_RDWR | O_APPEND, 0644);
				assert(logger->logFile >= 0);
			}
		}
		
		free(node->buf);
		free(node);	
	}
}

void logOpen(const char* logFile, int level)
{
	assert(logFile != NULL);

	logger = (struct Logger*)malloc(sizeof(struct Logger));
	assert(logger != NULL);

	strncpy(logger->filePath, logFile, 64);
	logger->logFile = open(logFile, O_CREAT | O_RDWR | O_APPEND, 0644);
	assert(logger->logFile >= 0);

	logger->level = level;
	logger->queue = newLogQueue();
	logger->run = true;
	
	logger->thresh = DEFAULT_THRESH;
	logger->logCount = 0;
	logger->fileCount = 0;

	pthread_create(&logger->tid, NULL, logThread, NULL);
}

void logClose()
{
	logger->run = false;
	
	logPrintf(LOG_WARN, "quit!\n");
}

void logPrintf(int level, const char* format, ...)
{
	char* buf = (char*)malloc(LOG_BUF_SIZE * sizeof(char));	
	assert(buf != NULL);

	va_list p;
	
	va_start(p, format);

	int n = vsnprintf(buf, LOG_BUF_SIZE - 1, format, p);
	assert(n > 0 && n < LOG_BUF_SIZE);

	va_end(p);

	pushLog(logger->queue, buf, n, level);
}


