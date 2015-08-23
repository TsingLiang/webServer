#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>

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

//connection map initial size
static const int CONN_INIT_SIZE = 1024;
//global variable
static struct httpServer* server = NULL;


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
static void sendError(struct httpConnection* conn);
static char** makeArgp();
static char** makeEnvp();

static void onResponse(struct BufferEvent* bevent, void* arg);

static void onRequest(struct BufferEvent* bevent, void* arg)
{
	assert(bevent != NULL);

	int connfd = bevent->event->fd;
	struct httpConnection* conn = server->connMap[connfd];

	assert(conn != NULL);

	while(conn->state != DISCONNECTED)
	{
		switch(conn->state)
		{
			case PARSE_FIRSTLINE:
				parseFirstLine(conn);
				break;
			
			case PARSE_HEADER:
				parseHeader(conn);
				break;

			case ADD_HEADER:
				addHeader(conn);
				break;

			case DO_FILE:
				doFile(conn);
				break;

			case DO_DIR:
				doDir(conn);
				break;
			
			case DO_CGI:
				doCgi(conn);
				break;

			case HTTP_ERROR:
				sendError(conn);
				break;

			default:
				break;
		}
	}	
}

static void onConnection(struct BufferEvent* bevent, void* arg)
{
	assert(bevent != NULL);

	int connfd = bevent->event->fd;
	char local[25];
	char peer[25];
	
	LogDebug("%s -> %s\n", getPeerAddr(connfd, peer, sizeof(peer)),
						getLocalAddr(connfd, local, sizeof(local)));
	
	assert(server != NULL);
	
	struct httpConnection* conn = newConnection(connfd, bevent, 
											server, server->loop);
	assert(conn != NULL);
	if(connfd >= server->mapSize)
	{
		struct httpConnection** newMap = (struct httpConnection*)realloc
								(server->connMap, server->mapSize * 2);
		assert(newMap != NULL);
		
		server->connMap = newMap;
	}
	server->connMap[connfd] = conn;	
}

static void parseFirstLine(struct httpConnection* conn)
{
	assert(conn != NULL);	

	struct Buffer* buffer = conn->bevent->input;
	assert(buffer != NULL);

	char* line = readLine(buffer);
	struct httpRequest* request = conn->request;
	assert(request != NULL);
	
	char* p = line;
	request->method = line;
	p = strchr(line, ' ');
	if(p == NULL)
	{
		conn->state = HTTP_ERROR;
		conn->errorCode = BAD_REQUEST;
		
		return;
	}
	
	while(*p == ' ')
		*p++ = '\0';
	
	char* method = request->method;
	if(	strcasecmp(method, "PUT") == 0
	 || strcasecmp(method, "TRACE") == 0
	 || strcasecmp(method, "OPTIONS") == 0
	 || strcasecmp(method, "DELETE") == 0)
	{
		conn->state = SEND_ERROR;
		conn->errorCode = NOT_IMPLEMENTED;
		
		return;
	}
	else if(strcasecmp(method, "GET") != 0
		 && strcasecmp(method, "HEAD") != 0
		 && strcasecmp(method, "POST") != 0)
	{
		conn->state = SEND_ERROR;
		conn->errorCode = BAD_REQUEST;
		
		return ;
	}

	request->url = p;
	p = strchr(p, ' ');
	if(p == NULL)
	{
		conn->state = HTTP_ERROR;
		conn->errorCode = BAD_REQUEST;
		
		return;
	}

	while(*p == ' ')
		*p++ = '\0';

	request->query = strchr(request->url, '?');
	if(request->query != NULL)
		*request->query++ = '\0';

	request->protocol = p;
	p = strchr(request->protocol, '.');
	if(p == NULL)
	{
		conn->state = HTTP_ERROR;
		conn->errorCode = BAD_REQUEST;
		
		return;
	}
	
	conn->state = PARSE_HEADER; 	
}

static void parseHeader(struct httpConnection* conn)
{
	assert(conn != NULL);	

	struct Buffer* buffer = conn->bevent->input;
	assert(buffer != NULL);

	char* line = readLine(buffer);
	struct httpRequest* request = conn->request;
	assert(request != NULL);
	
	char* line;
	char* p;

	while( (line = readLine(buffer)) != NULL)
	{
		if(line[0] = '\0')
			break;

		if(strncasecmp(line, "Authorization:", 14) == 0)
		{
			p = &line[14];	
			while(*p == ' ')
				*p = '\0';

			request->authrozation = p;
		}
		else if(strncasecmp(line, "Content-Length:", 15) == 0)
		{
			p = &line[15];
			while(*p == ' ')
				*p = '\0';

			request->contentLength = atol(p);
		}
		else if(strncasecmp(line, "Content-Type:", 13) == 0)
		{
			p = &line[13];
			while(*p == ' ')
				*p = '\0';

			request->contentType = p;	
		}
		else if(strncasecmp(line, "Cookie:", 7) == 0)
		{
			p = &line[7];
			while(*p == ' ')
				*p++ = '\0';

			request->cookie = p;
		}
		else if(strncasecmp(line, "Host:", 5) == 0)
		{
			p = &line[5];
			while(*p == ' ')
				*p++ = '\0';

			request->host = p;
			if(strchr(reqeust->host, '/') != NULL 
				|| request->host[0] == '.')
			{
				conn->state = HTTP_ERROR;
				conn->errorCode = BAD_REQUEST;

				return ;
			}
		}
		else if(strncasecmp(line, "Referer:", 8) == 0)
		{
			p = &line[8];
			while(*p == ' ')
				*p++ = '\0';

			request->referrer = p;
		}
		else if(strncasecmp(line, "Referrer, 9") == 0)
		{
			p = &line[9];
			while(*p == ' ')
				*p++ = '\0';

			request->referrer = p;
		}
		else if(strncasecmp(line, "User-Agent:", 11) == 0)
		{
			p = &line[11];
			while(*p == ' ')
				*p = '\0';

			request->userAgent = p;
		}
		else if(strnacasecmp(line, "If-Modified-Since:", 18) == 0)
		{
			p = &line[18];
			while(*p == ' ')
				*p++ = '\0';
			
			reqeust->modified = p;
		}
		else if(strncasecmp(line, "Accept:", 7) == 0)
		{
			p = &line[7];
			while(*p == ' ')
				*p++ = '\0';
			
			request->accept = p;
		}
		else if(strncasecmp(line, "Accept-Language:", 16) == 0)
		{
			p = &line[16];
			while(*p == ' ')
				*p++ = '\0';

			request->acceptLanguage = p;
		}
	}
			
}

void addHeader(struct httpConnection* conn)
{
	assert(conn != NULL);

	struct httpRequest* request = conn->request;
	assert(request != NULL);
	
	char* method = request->method;
	assert(method != NULL);
	if(strcasecmp(method, "GET") == 0)
	{
		struct 		
	}
	else if(strcasecmp(method, "HEAD") == 0)
	{

	}
	else if(strcasecmp(method, "POST") == 0)
	{
	}
	else
	{
		conn->state = SEND_ERROR;
		conn->errorCode = BAD_REQUEST;
	}
}

static void doFile(struct httpConnection* conn);
static void doDir(struct httpConnection* conn);
static void doCgi(struct httpConnection* conn);
static void sendError(struct httpConnection* conn);
static char** makeArgp();
static char** makeEnvp();

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
	conn->state = PARSE_HEADER;
	conn->request = (struct httpRequest* request)malloc
						(sizeof(struct httpRequest));
	assert(conn->request != NULL);

	return conn;
}

void freeConnection(struct httpConnection* conn)
{
		
}

static const int CONN_INIT_SIZE = 1024;
static struct httpServer* server = NULL;

void newHttpServer(int argc, char* argv[])
{
	server = (struct httpServer*)malloc(sizeof(struct httpServer));
	assert(server != NULL);

	server->setting = parseOpt(argvc, argv);
	assert(server->setting != NULL);

	logOpen(server->setting->logFile, server->setting->logLevel);

	struct EventLoop* loop = (struct EventLoop*)malloc
								(sizeof(struct EventLoop));
	assert(loop != NULL);
	eventLoopInit(loop);	

	server->server = newServer(loop, server->setting->listen,
								server->setting->serverName, 
								strlen(erver->setting->serverName));
	assert(server->server != NULL);
	server->acceptor = server->server->sockfd;
	
	setThreadNumber(server->server, server->setting->nthreads);
	
	server->connMap = (struct httpConnection**)malloc
						(CONN_INIT_SIZE * sizeof(struct httpConnection*));
	assert(server->connMap != NULL);
	server->mapSize = CONN_INIT_SIZE;

	setAcceptCb(server->server, onConnection);
	setReadCb(server->server, OnRequest);
	setWriteCb(server->server, OnResponse);	
}

void startServer()
{

	if(server->daemon)
	{
#ifdef DEBUG
	printf("run as a daemon!\n");
#endif
		daemon();
		singleRun(server->setting->pidFile);
	}

	if(getuid() == 0)
	{
		struct passwd* pwd;

		pwd = getpwent("nobody");
		if(pwd == NULL)
		{
			LogError("user %s is not exist.\n", "nobody");
			exit(0);
		}

		int uid = pwd->pw_uid;
		int gid = pwd->pw_gid;

		if(chown(setting->logFile, uid, gid) < 0)
		{
			extern int errno;
			LogError("logfile chown() error: %s\n", strerror(errno));
		}
		
		if(chown(setting->pidFile, uid, gid) < 0)
		{
			extern int errno;
			LogError("logfile chown() error: %s\n", strerror(errno));
		}

		if(chroot(setting->root) < 0)
		{
			perror("chroot() error:");
			exit(0);
		}
	
		LogDebug("now root = %s\n", setting->root);
		
		if(setuid() < 0)
		{
			LogError("setuid() error: %s\n", strerror(errno));
			exit(0);
		}
		
		if(setgid() < 0)
		{
			LogError("setgid() error: %s\n", strerror(errno));
			exit(0);
		}
	}

	start(server->server);		
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


