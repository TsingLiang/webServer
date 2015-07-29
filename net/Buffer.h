#ifndef BUFFER_H
#define BUFFER_H

#define DEBUG

/*
....abcdefghijk.......................
buf		       windex
	rindex			
*/
	
struct Buffer
{
	char* buf;
	int rindex;
	int windex;
	int capacity;
};

struct Buffer* newBuffer();
int bufferRead(struct Buffer* buffer, int fd);
int bufferAddStr(struct Buffer* buffer, const void* data, int len);
int bufferAddInt(struct Buffer* buffer, int n);
int bufferWrite(struct Buffer* buffer, int fd);
char* readLine(struct Buffer* buffer);

void freeBuffer(struct Buffer* buffer);


#endif
