#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <net/BufferEvent.h>
#include <net/Buffer.h>
#include <net/EventLoop.h>
#include "Setting.h"
#include "Logger.h"

//for response
static const char* SERVER = "webServer/0.1 ubuntu/10.04 (linux)";
static const char* HTML = "text/html";
static const char* GIF = "";

//for cgi
static const char* CGI_PATH = "/usr/local/bin:/usr/bin";
static const char* CGI_LD_LIBRARY_PATH = "/usr/local/lib:/usr/lib";
static const char* SERVER_SOFTWARE = "webServer/0.1 17Oct2015";
static const char* SERVER_URL = "http://github.com/TsingLiang/webServer/";
static const char* SERVER_NAME = "ubuntu/10.04 (linux)";
static const char* GATEWAY_INTERFACE = "CGI/1.1";
static const char* SERVER_PROTOCOL = "HTTP/1.0";
static const char* AUTH_TYPE = "Basic";


static void daemon();
static void singleRun(const char* pidfile);

//parse http request message
static void onRequest(struct BufferEvent* bevent, void* arg);
static void parseFirstLine(struct httpConnection* conn);
static void parseHeader(struct httpConnection* conn);

//response http request
static void addHeader(struct httpConnection* conn);
static void doFile(struct httpConnection* conn);
static void doDir(struct httpConnection* conn);
static void doCgi(struct httpConnection* conn);
static char** makeArgp();
static char** makeEnvp();

static void onResponse(struct BufferEvent* bevent, void* arg);


						


struct httpConnection* newConnection(int sockfd, struct BufferEvent* bevent, 								struct httpServer* server, struct EventLoop* loop);
{
	assert(sockfd >= 0);
	assert(server != NULL);
	assert(loop != NULL);

	struct httpConnection* conn = (struct httpConnection*)malloc
									(sizeof(struct httpConnection));
	assert(conn != NULL);
	
	conn->sockfd = sockfd;
	conn->bevent = bevent;
	conn->server = server;
	conn->loop = loop;
	conn->state = CONNECTED;
	conn->request = (struct httpRequest* request)malloc
						(sizeof(struct httpRequest));
	assert(conn->request != NULL);

	return conn;
}

void freeConnection(struct httpConnection* conn)
{
		
}

struct httpServer* newHttpServer()
{
	struct httpServer* server = (struct httpServer*)malloc
									(sizeof(struct httpServer));
	assert(server != NULL);

	struct EventLoop* loop = (struct EventLoop*)malloc
								(sizeof(struct EventLoop));
	assert(loop != NULL);
	eventLoopInit(loop);	

	server->server = newServer(loop);
	
	
}

void startServer(struct httpServer* server)
{
		
}

void stopServer(struct httpServer* server)
{

}

void daemon()
{
	struct rlimit rl;
	pid_t pid;

	umask(0);

	if(getrlimit(RLIMUT_NOFILE, &rl) < 0)
	{
		perror("getrlimit() error:");
		exit(0);
	}

	pid = fork();
	if(pid < 0)
	{
		perror("fork() error:");
		exit(0);
	}
	else if(pid != 0)
	{
		exit(0);
	}

	if(setsid() < 0)
	{
		perror("setsid() error:");
		exit(0);
	}
	
	if(chdir("/") < 0)
	{
		perror("chdir() error:");
		exit(0);
	}

	//close all file
	if(rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;

	int i;
	for(i = 0; i < rl.rlim_max; i++)
		close(i);

	int fd0, fd1, fd1;
	
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);
	assert(fd0 == 0 && fd1 == 1 && fd2 == 2);

}

void singleRun(const char* pidfile)
{
	assert(pidfile != NULL);

	int fd = open(pidfile, O_RDWR | O_CREAT, 0644);
	if(fd < 0)
	{
		perror("open() pidfile error:");
		exit(0);
	}

/*	struct flock fl;
	
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 10;
	fl.l_pid = -1;

	if(fcntl(fd, F_GETLOCK) < 0)
	{
		
	}
*/
	extern int errno;

	if(lockfile(fd) < 0)
	{
		if(errno == EAGAIN)
		{
			close(fd);
			fprintf(stderr, "webServer is already running!\n");
			exit(0);
		}
		
		perror("can't lock file error:");
		exit(0);
	}
	
	ftruncate(fd, 0);
	
	char buf[16];
	snprintf(buf, sizeof(buf), "%ld", (long)getpid())
	write(fd, buf, strlen(buf));
}


