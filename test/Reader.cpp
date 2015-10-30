#include "Reader.h"
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

Reader::~Reader()
{}

BufferedReader::BufferedReader()
{
	begin = 0;
	end = 0;

	buffer = (char*)malloc( (BUFFER_SIZE + 1) * sizeof(char));
	assert(buffer != NULL);
}

BufferedReader::BufferedReader(const char file[])
{
	begin = 0;
	end = 0;

	buffer = (char*)malloc( (BUFFER_SIZE + 1) * sizeof(char));
	assert(buffer != NULL);

	assert(open(file) == true);
}

BufferedReader::~BufferedReader()
{
	::free(buffer);
}

bool BufferedReader::open(const char file[])
{
	fd = ::open(file, O_RDONLY);

	return fd >= 0;
}

int BufferedReader::read()
{
	if(begin == end)
	{
		begin = 0;
		end = 0;

		int n = ::read(fd, buffer, BUFFER_SIZE);
		if(n == 0)
			return -1;

		end += n;
	}

	return buffer[begin++];
}
int BufferedReader::read(char buf[], int off, int len)
{
	if(begin == end)
	{
		begin = 0;
		end = 0;

		int n = ::read(fd, buffer, BUFFER_SIZE);
		if(n == 0)
			return -1;

		end += n;
	}
	
	int have = end - begin;
	int n = len <= have ? len : have;
	strncpy(buf + off, buffer + begin, n);
	begin += n;

	return n;
}
void BufferedReader::lseek(int off, int whence)
{
	::lseek(fd, off, whence);
}
void BufferedReader::close()
{
	::close(fd);
}

