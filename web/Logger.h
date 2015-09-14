#ifndef LOGGER_H
#define LOGGER_H

#include <pthread.h>
#include <stdbool.h>


#define DEBUG

enum LogLevel
{
	LOG_DEBUG,
	LOG_INFO,
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

	char filePath[64];
	int logFile;
	int level;
	struct LogQueue* queue;

	int thresh;
	int logCount;
	int fileCount;
};

void logOpen(const char* logFile, int level);
void logClose();
void logPrintf(int level, const char* format, ...);


#define LogDebug(format, args...)		\
	logPrintf(LOG_DEBUG, format, ##args)
#define LogInfo(format, args...)		\
	logPrintf(LOG_INFO, format, ##args)
#define LogWarn(format, args...)		\
	logPrintf(LOG_WARN, format, ##args)
#define LogError(format, args...)		\
	logPrintf(LOG_ERROR, format, ##args)

#endif
