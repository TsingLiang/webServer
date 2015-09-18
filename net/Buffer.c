#include "Buffer.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

static const int BUFFER_SIZE = 1024;

static bool bufferExpand(struct Buffer* buffer)
{
	if(buffer == NULL)
		return;

	int oldSize = buffer->capacity;
	char* newBuf = (char*)realloc(buffer->buf, oldSize * 2);
	
	if(newBuf != NULL)
	{
		buffer->buf = newBuf;
		buffer->capacity = oldSize * 2;

		return true;
	}

	return false;
}

struct Buffer* newBuffer()
{
	struct Buffer* buffer = (struct Buffer*)malloc(sizeof(struct Buffer));	

	if(buffer != NULL)
	{
		buffer->buf = (char*)malloc(BUFFER_SIZE * sizeof(char));
		if(buffer->buf != NULL)
		{
			buffer->rindex = 0;
			buffer->windex = 0;
			buffer->capacity = BUFFER_SIZE;
		}
	}

	return buffer;
}

int bufferRead(struct Buffer* buffer, int fd)
{
	if(buffer == NULL || fd < 0)
		return -1;

	int nread = 0;
	int sum = 0;
	int nhave = buffer->capacity - buffer->windex;
	while( (nread = read(fd, buffer->buf + buffer->windex, nhave)) == nhave)
	{
		sum += nread;
		buffer->windex += nread;
		if(bufferExpand(buffer) == false)
		{
			break;
		}
	}
	
	if(sum > 0)
		return sum;
	
	buffer->windex += nread;

	return nread;	
}

int bufferAddStr(struct Buffer* buffer, const void* data, int len)
{		
	if(buffer == NULL || data == NULL || len <= 0)
		return -1;

	int nhave = buffer->capacity - buffer->windex;
	while(nhave < len)
	{
		if(bufferExpand(buffer) == false)
			return 0;
		
		nhave = buffer->capacity - buffer->windex;
	}

	memcpy(buffer->buf + buffer->windex,  data, len);
	buffer->windex += len;

	return len;
}


int bufferPrintf(struct Buffer* buffer, const char* fmt, ...)
{
	assert(buffer != NULL);

	va_list p;
	va_start(p, fmt);

	int n = vsnprintf(buffer->buf + buffer->windex, 
					buffer->capacity - buffer->windex, fmt, p);

	if(n < 0)
		return n;

	buffer->windex += n;
	
	int sum = n;
	
	while(buffer->windex == buffer->capacity)
	{
		bufferExpand(buffer);

		n = vsnprintf(buffer->buf + buffer->windex, 
					buffer->capacity - buffer->windex, fmt, p);	
		if(n < 0)
			return n;

		buffer->windex += n;

		sum += n;
	}

	va_end(p);

	return sum;
}

int bufferAddInt(struct Buffer* buffer, int n)
{
	if(buffer == NULL)
		return -1;

	char buf[4];
	snprintf(buf, 4, "%d", n);
	bufferAddStr(buffer, buf, sizeof(buf));	

	return sizeof(int);
}


int bufferWrite(struct Buffer* buffer, int fd)
{
	if(buffer == NULL || fd < 0)
		return -1;

	int nwrite = write(fd, buffer->buf + buffer->rindex, 
					buffer->windex - buffer->rindex);
	if(nwrite > 0)
		buffer->rindex += nwrite;

	return nwrite;
}

void bufferSwap(struct Buffer* buffer1, struct Buffer* buffer2)
{
	if(buffer1 == NULL || buffer2 == NULL)
		return;

	char* buf = buffer1->buf;
	buffer1->buf = buffer2->buf;
	buffer2->buf = buf;

	int rindex = buffer1->rindex;
	buffer1->rindex = buffer2->rindex;
	buffer2->rindex = rindex;
	
	int windex = buffer1->windex;
	buffer1->windex = buffer2->windex;
	buffer2->windex = windex;
	
	int capacity = buffer1->capacity;
	buffer1->capacity = buffer2->capacity;
	buffer2->capacity = capacity;
}


char* readLine(struct Buffer* buffer)
{
	if(buffer == NULL || buffer->rindex >= buffer->windex)
		return NULL;

	char *start = buffer->buf + buffer->rindex;
	char *p = start;
	while(*p != '\n' && *p != '\r' && *p != '\0')
	{
		p++;
		buffer->rindex++;
	}

	if(*p == '\n')	
	{
		*p = '\0';
		buffer->rindex++;
	}
	else if(*p == '\r')
	{
		*p = '\0';
		buffer->rindex += 2;
	}
	else
	{
		buffer->rindex++;
	}

	return start;
}

void freeBuffer(struct Buffer* buffer)
{
	if(buffer != NULL)
	{
		if(buffer->buf != NULL)
			free(buffer->buf);

		free(buffer);
	}
}

