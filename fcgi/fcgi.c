#include "fcgi.h"
#include <net/Buffer.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

static int count = 0;
static bool is_init = false;

static void fcgi_init(int argc, char* argv[]);

int fcgi_accept(int argc, char* argv[])
{
//	fprintf(stderr, "count = %d\n", ++count);
	
	if(!is_init)
		fcgi_init(argc, argv);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);

	int connfd = tcpAccept(NEW_FCGI_LISTEN_FD);
//	fprintf(stderr, "connfd = %d\n", connfd);
	assert(connfd >= 0);
	setNonBlock(connfd, false);

	struct Buffer* input = newBuffer();

//	fprintf(stderr, "before read\n");
	int n = bufferRead(input, connfd);
	fprintf(stderr, "n = %d\n", n);


//	fprintf(stderr, "after read\n");
	char* line;

//init environment
	while( (line = readLine(input)) != NULL && line[0] != '\0')
	{
		if(strlen(line) > 0)
		{
//			fprintf(stderr, "%s\n", line);
			putenv(line);
		}
	}
	
	freeBuffer(input);

	dup2(connfd, STDIN_FILENO);
	dup2(connfd, STDOUT_FILENO);

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
//	fprintf(stderr, "fcgi accept over. \n");

	return 0;
}

void fcgi_init(int argc, char* argv[])
{
/*	if(argc != 2)
	{
		printf("fcgi_init() error: argc != 2.\n");
		exit(0);
	} */

	dup2(FCGI_LISTEN_FD, NEW_FCGI_LISTEN_FD);

	is_init = true;
}
