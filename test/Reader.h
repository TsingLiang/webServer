#ifndef READER_H
#define READER_H

class Reader
{
public:
	virtual ~Reader() = 0;
	virtual bool open(const char file[]) = 0;
	virtual int read() = 0;
	virtual int read(char buf[], int off, int len) = 0;
	virtual void lseek(int off, int whence) = 0;
	virtual void close() = 0;
};

class BufferedReader: public Reader
{
public:
	static const int BUFFER_SIZE = 4096;

	BufferedReader();
	BufferedReader(const char file[]);
	~BufferedReader();

	bool open(const char file[]);
	int read();
	int read(char buf[], int off, int len);
	void lseek(int off, int whence);
	void close();
private:
	int fd;
	char* buffer;
	int begin;
	int end;
};

#endif
