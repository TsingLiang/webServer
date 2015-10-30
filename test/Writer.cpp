#include "Writer.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

Writer::~Writer()
{}

BufferedWriter::BufferedWriter()
{
	size = 0;
	buffer = (char*)malloc(BUFFER_SIZE * sizeof(char));
	assert(buffer != NULL);
}
BufferedWriter::BufferedWriter(char file[])
{
	size = 0;
	buffer = (char*)malloc(BUFFER_SIZE * sizeof(char));
	assert(buffer != NULL);

	assert(open(file) == true);
}
BufferedWriter::BufferedWriter(char file[], bool append)
{
	size = 0;
	buffer = (char*)malloc(BUFFER_SIZE * sizeof(char));
	assert(buffer != NULL);

	assert(open(file, true) == true);
}
BufferedWriter::~BufferedWriter()
{
	::free(buffer);
}

bool BufferedWriter::open(char file[])
{
	fd = ::open(file, O_WRONLY);

	return fd >= 0;
}

bool BufferedWriter::open(char file[], bool append)
{
	if(append)
		fd = ::open(file, O_WRONLY | O_APPEND);
	else
		fd = ::open(file, O_WRONLY);

	return fd >= 0;
}
void BufferedWriter::write(int c)
{
	if(size == BUFFER_SIZE)
	{
		flush();

		size = 0;
	}

	buffer[size++] = (char)c;
}
void BufferedWriter::write(char buf[], int off, int len)
{
	int have = BUFFER_SIZE - size;

	if(len <= have)
	{
		strncpy(buffer + size, buf + off, len);
		size += len;
	}
	else if(len < BUFFER_SIZE)
	{
		flush();
		strncpy(buffer + size, buf + off, len);
		size += len;
	}
	else
	{
		int direct = len / BUFFER_SIZE * BUFFER_SIZE;

		flush();
		::write(fd, buf + off, direct);
		off += direct;
		len -= direct;
	
		strncpy(buffer + size, buf + off, len);
		size += len;
	}
}
void BufferedWriter::lseek(int off, int whence)
{
	::lseek(fd, off, whence);
}
void BufferedWriter::flush()
{
	printf("fd = %d, size = %d\n", fd, size);

	::write(fd, buffer, size);
	size = 0;
}
void BufferedWriter::close()
{
	flush();
	::close(fd);
}
