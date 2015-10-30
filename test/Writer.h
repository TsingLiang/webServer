#ifndef WRITER_H
#define WRITER_H

#include <stdlib.h>

class Writer
{
public:
	virtual ~Writer() = 0;
	virtual bool open(char file[]) = 0;
	virtual bool open(char file[], bool append) = 0;
	virtual void write(int c) = 0;
	virtual void write(char buf[], int off, int len) = 0;
	virtual void lseek(int off, int whence) = 0;
	virtual void flush() = 0;
	virtual void close() = 0;
};

class BufferedWriter: public Writer
{
public:
	static const int BUFFER_SIZE = 4096;

	BufferedWriter();
	BufferedWriter(char file[]);
	BufferedWriter(char file[], bool append);
	~BufferedWriter();

	bool open(char file[]);
	bool open(char file[], bool append);
	void write(int c);
	void write(char buf[], int off, int len);
	void lseek(int off, int whence);
	void flush();
	void close();

private:
	int fd;
	char* buffer;
	int size;
};

#endif
