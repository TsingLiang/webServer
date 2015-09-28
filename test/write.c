#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>

#define FILE_NAME "./test.txt"

int main()
{
	int fd = open(FILE_NAME, O_WRONLY | O_APPEND);
	assert(fd >= 0);

	int ret = lseek(fd, 1, SEEK_SET);
	assert(ret >= 0);
	char buf[] = "writed text.";
	
	write(fd, buf, strlen(buf));

	close(fd);
}
