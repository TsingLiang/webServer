#include <stdio.h>
#include <net/Socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define FCGI_LISTEN_FD 0

static void spawn(const char* cgi, char* argv[]);
static void usage();

int main(int argc, char* argv[])
{
	short port = 0;
	char* cgi = NULL;
	int   count = 0;
	int c;

	while( (c = getopt(argc, argv, "p:c:f:h")) != -1)
	{
		switch(c)
		{
			case 'h': 
				usage();
				exit(0);

			case 'p':
				port = atoi(optarg);
				break;

			case 'c':
				count = atoi(optarg);
				break;

			case 'f':
				cgi = strdup(optarg);
				break;

			default:
				usage();
				exit(0);
		}
	}

	if(port == 0 || cgi == NULL || count == 0)
	{
		usage();
		exit(0);
	}

	int listener = tcpListen(port);
	setNonBlock(listener, false);
	dup2(listener, FCGI_LISTEN_FD);

	int i;
	for(i = 0; i < count; i++)
		spawn(cgi, argv);

	printf("spawn() %s success.\n", cgi);

	return 0;
}

void usage()
{
	printf("this is spawn 0.1, to use it.\n");
	printf("-h		help\n");
	printf("-p		listen port\n");
	printf("-f		cgi to spawn\n");
	printf("-c 		the number of cgi to spawn.\n");
}

void spawn(const char* cgi, char* argv[])
{
	pid_t pid = fork();

	if(pid < 0)
	{
		perror("spawn() error:");
		exit(0);
	}
	//child
	else if(pid == 0)
	{
		execv(cgi, argv);
	}
	//parent
}
