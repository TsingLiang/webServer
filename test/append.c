#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#define FILE_NAME "./test.txt"

int main()
{
	int fd = open(FILE_NAME, O_RDONLY | O_APPEND);
	assert(fd >= 0);

	int ret = lseek(fd, 1, SEEK_SET);
	assert(ret >= 0);
	char buf[BUFSIZ];
	
	int n = read(fd, buf, sizeof(buf));
	assert(n >= 0);
	write(STDOUT_FILENO, buf, n);

	close(fd);
}
