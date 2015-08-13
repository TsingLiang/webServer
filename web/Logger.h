#ifndef LOGGER_H
#define LOGGER_H

#include <pthread.h>
#include <stdbool.h>


#define DEBUG

enum LogLevel
{
	LOG_DEBUG,
	LOG_WARN,
	LOG_ERROR
};

struct LogNode
{
	char* buf;
	int   len;
	int   level;
	struct LogNode* next;
};

struct LogQueue
{
	struct LogNode* head;
	struct LogNode*	tail;

	pthread_mutex_t lock;
	pthread_cond_t  cond;
};

struct Logger
{
	pthread_t tid;
	bool run;

	int logFile;
	int level;	
	struct LogQueue* queue;
};

void logOpen(const char* logFile, int level);
void logClose();
void logPrintf(int level, const char* format, ...);


#define LogDebug(format, args...)		\
	logPrintf(DEBUG, format, ##args)
#define LogWarn(format, args...)		\
	logPrintf(WARN, format, ##args)
#define LogError(format, args...)		\
	logPrintf(ERROR, format, ##args)

#endif
