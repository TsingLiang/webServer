#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <sys/resource.h>
#include <sys/stat.h>

#include <net/BufferEvent.h>
#include <net/Buffer.h>
#include <net/Event.h>
#include <net/EventLoop.h>
#include <net/Server.h>
#include <net/Socket.h>
#include "webServer.h"
#include "Setting.h"
#include "Logger.h"

//for response
static const char* HTTP_VERSION = "HTTP/1.0";
static const char* SERVER = "webServer/0.1 ubuntu/10.04 (linux)";
static const char* HTML = "text/html";
static const char* GIF = "gif";

//for cgi
static const char* CGI_PATH = "/usr/local/bin:/usr/bin";
static const char* CGI_LD_LIBRARY_PATH = "/usr/local/lib:/usr/lib";
static const char* SERVER_SOFTWARE = "webServer/0.1 17Oct2015";
static const char* SERVER_URL = "http://github.com/TsingLiang/webServer/";
static const char* SERVER_NAME = "ubuntu/10.04 (linux)";
static const char* GATEWAY_INTERFACE = "CGI/1.1";
static const char* SERVER_PROTOCOL = "HTTP/1.0";
static const char* AUTH_TYPE = "Basic";

//state to string
static const char* states[] = {
								"disconnected"
								, "connected"
								, "parse first line"
								, "parse header"
								, "pre response"
								, "do file"
								, "do dir"
								, "do cgi"
								, "on cgi"
								, "send response"
								, "send error"
												};

//errorCode to string
static const char* errorCodes[] = {
									  "http ok"
									, "bad request"
									, "not found"
									, "not implemented"
									, "forbidden"
									, "internal error"
													};

//connection map initial size
static const int CONN_INIT_SIZE = 1024;
//http server 
static struct httpServer* server = NULL;

static void daemonize();
static void singleRun(const char* pidfile);

static void onConnection(struct BufferEvent* bevent, void* arg);
//parse http request message
static void onRequest(struct BufferEvent* bevent, void* arg);
static void parseFirstLine(struct httpConnection* conn);
static void parseHeader(struct httpConnection* conn);

//response http request
static void preResponse(struct httpConnection* conn);
static void addGeneralHeader(struct httpConnection* conn);
static void doFile(struct httpConnection* conn);
static void doDir(struct httpConnection* conn);
static void doCgi(struct httpConnection* conn);
static void onCgi(struct BufferEvent* cevent, void* arg);
static void sendResponse(struct httpConnection* conn);
static void sendError(struct httpConnection* conn);
static char** makeArg(struct httpConnection* conn);
static char** makeEnv(struct httpConnection* conn);

static void onResponse(struct BufferEvent* bevent, void* arg)
{}

static void onRequest(struct BufferEvent* bevent, void* arg)
{
	assert(bevent != NULL);

	int connfd = bevent->event->fd;
	struct httpConnection* conn = server->connMap[connfd];

	assert(conn != NULL);

	while(conn->state != DISCONNECTED && conn->state != ON_CGI)
	{

#ifdef DEBUG
		printf("connection state: %s.\n", states[conn->state]);
#endif
		switch(conn->state)
		{
			case PARSE_FIRSTLINE:
				parseFirstLine(conn);
				break;
			
			case PARSE_HEADER:
				parseHeader(conn);
				break;

			case PRE_RESPONSE:
				preResponse(conn);
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

			case SEND_RESPONSE:
				sendResponse(conn);
				break;

			case SEND_ERROR:
				sendError(conn);
				break;

			default:
				break;
		}
	}
	
	if(conn->state == DISCONNECTED)
		freeConnection(conn);
	
#ifdef DEBUG
	printf("connection state: %s\n", states[conn->state]);
#endif	

}

void onConnection(struct BufferEvent* bevent, void* arg)
{
	assert(bevent != NULL);

	int connfd = bevent->event->fd;
	char local[25];
	char peer[25];

#ifdef DEBUG
	printf("new connection:%s -> %s\n", getPeerAddr(connfd, peer, sizeof(peer)),
						getLocalAddr(connfd, local, sizeof(local)));
#endif
	
	LogDebug("%s -> %s\n", getPeerAddr(connfd, peer, sizeof(peer)),
						getLocalAddr(connfd, local, sizeof(local)));
	
	assert(server != NULL);
	
	struct httpConnection* conn = newConnection(connfd, bevent, 
											server, server->server->loop);
	assert(conn != NULL);
	if(connfd >= server->mapSize)
	{
		struct httpConnection** newMap = (struct httpConnection**)realloc
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
		conn->state = SEND_ERROR;
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

#ifdef DEBUG
	printf("method: %s\n", request->method);
#endif

	request->url = p;
	p = strchr(p, ' ');
	if(p == NULL)
	{
		conn->state = SEND_ERROR;
		conn->errorCode = BAD_REQUEST;
		
		return;
	}

	while(*p == ' ')
		*p++ = '\0';

#ifdef DEBUG
	printf("url: %s\n", request->url);
#endif

	request->query = strchr(request->url, '?');
	if(request->query != NULL)
		*request->query++ = '\0';

	request->protocol = p;
	p = strchr(request->protocol, '.');
	if(p == NULL)
	{
		conn->state = SEND_ERROR;
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

			request->authorization = p;
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
			if(strchr(request->host, '/') != NULL 
				|| request->host[0] == '.')
			{
				conn->state = SEND_ERROR;
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
		else if(strncasecmp(line, "Referrer", 9) == 0)
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
		else if(strncasecmp(line, "If-Modified-Since:", 18) == 0)
		{
			p = &line[18];
			while(*p == ' ')
				*p++ = '\0';
			
			request->modified = p;
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
	
	conn->state = PRE_RESPONSE;		
}

void preResponse(struct httpConnection* conn)
{
	assert(conn != NULL);

	struct httpRequest* request = conn->request;
	assert(request != NULL);
	
	char* url = request->url;
	char  path[256];
	struct Setting* setting = server->setting;
	assert(setting != NULL);

	int n = strlen(setting->document);
	if(setting->document[n - 1] == '/')
		setting->document[n - 1] = '\0';
	
	snprintf(path, sizeof(path) - 1, "%s%s%s", setting->root, 
							setting->document, url);
	
#ifdef DEBUG
	printf("path: %s\n", path);
#endif
		
	struct stat st;
			
	if(stat(path, &st) < 0)
	{
		conn->state = SEND_ERROR;
		conn->errorCode = NOT_FOUND;
			
		return;
	}
	
	request->path = strdup(path);
	
	if(S_ISDIR(st.st_mode))
	{
		conn->state = DO_DIR;
			
		return;
	}

//*.cgi || *.fcgi
	char* p = path + strlen(path);
	if(	*--p == 'i' && *--p == 'g' && *--p == 'c' && 
		(*--p == '.' || (*p == 'f' && *--p == '.')))
	{
		conn->state = DO_CGI;
			
		return;
	}

	conn->state = DO_FILE;
}

void addGeneralHeader(struct httpConnection* conn)
{
	assert(conn != NULL);

	struct Buffer* output = conn->bevent->output;
	assert(output != NULL);

	bufferPrintf(output, "Server: %s\r\n", SERVER);
	bufferPrintf(output, "Accept: %s;%s\r\n", HTML, GIF);
		
	time_t now = time(NULL);
	char* timeStr = ctime(&now);	
	
	bufferPrintf(output, "Date: %s\r\n", timeStr);
	
	//free(timeStr);
}

void doFile(struct httpConnection* conn)
{
	assert(conn != NULL);

	struct httpRequest* request = conn->request;
	assert(request != NULL);
	
	char* url = request->url;
	char* path = request->path;

	struct stat st;
			
	if(stat(path, &st) < 0)
	{
		extern int errno;

		conn->state = SEND_ERROR;
		conn->errorCode = NOT_FOUND;
			
		LogError("stat() %s error: %s\n", strerror(errno));

		return;
	}

	int fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		extern int errno;
		LogError("open() %s error: %s\n", strerror(errno));

		conn->state = SEND_ERROR;
		conn->errorCode = FORBIDDEN;
		
		return;
	}
	
	struct Buffer* output = conn->bevent->output;
	assert(output != NULL);
	bufferPrintf(output, "%s 200 OK\r\n", HTTP_VERSION);
	bufferPrintf(output, "Content-length: %u\r\n", st.st_size);
	bufferPrintf(output, "Content-type: text/html\r\n");
	addGeneralHeader(conn);
	
	bufferRead(output, fd);
	
	close(fd);	

	conn->state = SEND_RESPONSE;
}

void doDir(struct httpConnection* conn)
{
	assert(conn != NULL);

	struct httpRequest* request = conn->request;
	assert(request != NULL);
	
	char* url = request->url;
	char path[256];

	snprintf(path, sizeof(path) - 1, "%s/index.html", request->path);

	struct stat st;
			
	if(stat(path, &st) < 0)
	{
		conn->state = SEND_ERROR;
		conn->errorCode = NOT_FOUND;
			
		return;
	}

	int fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		extern int errno;
		LogError("open() %s error: %s\n", strerror(errno));

		conn->state = SEND_ERROR;
		conn->errorCode = FORBIDDEN;
		
		return;
	}
	
	struct Buffer* output = conn->bevent->output;
	assert(output != NULL);
	bufferPrintf(output, "%s 200 OK\r\n", HTTP_VERSION);
	bufferPrintf(output, "Content-length: %u\r\n", st.st_size);
	bufferPrintf(output, "Content-type: text/html\r\n");
	addGeneralHeader(conn);
	
	bufferRead(output, fd);
	
	close(fd);	

	conn->state = SEND_RESPONSE;	
}

void doCgi(struct httpConnection* conn)
{
	assert(conn != NULL);
	
	struct httpRequest* request = conn->request;
	assert(request != NULL);
	struct Setting* setting = server->setting;

	if(	strcasecmp(request->method, "GET") != 0
	 && strcasecmp(request->method, "POST") != 0)
	{
		conn->state = SEND_ERROR;
		conn->errorCode = NOT_IMPLEMENTED;	
		
		return;
	}
	
	char** env = makeEnv(conn);
	char*  args[2];
	args[0] = request->path;
	args[1] = NULL;

    if(setting->usefcgi)
    {
		char ip[25];
		int  port;
		bool error = false;
		int i; 
		for(i = 0; i < setting->size; i++)
		{
			int len = strlen(setting->location[i].file);
			if(strncmp(setting->location[i].file, request->path, len) == 0)
			{
				if(sscanf(setting->location[i].ipport, 
							"%s : %d", ip, &port) < 0)
					error = true;
				break;
			}
		}

		if(error || i == setting->size)
		{
			conn->state = SEND_ERROR;
			conn->errorCode = INTERNAL_ERROR;
	
			free(env);

			return;
		}

#ifdef DEBUG
		printf("ip = %s, port = %d\n", ip, port);
#endif
		int cgifd = tcpConnect(ip, port);
		if(cgifd < 0)
		{
			conn->state = SEND_ERROR;
			conn->errorCode = INTERNAL_ERROR;
	
			free(env);

			return;
		}
		struct Buffer* output = newBuffer();
		for(i = 0; i < 50; i++)
		{
			if(env[i] == NULL)
				break;
			bufferAddStr(output, env[i], strlen(env[i]));
			bufferAddStr(output, "\n", 1);
		}

		write(cgifd, output->buf, output->windex - output->rindex);
		freeBuffer(output);

 		if(strcasecmp(request->method, "POST") == 0)
			write(cgifd, request->query, strlen(request->query));
	
		conn->cevent = newBufferEvent(server->server->loop, cgifd,
								onCgi, NULL, conn);
		enableRead(conn->cevent);
	
		for(i = 0; i < 50; i++)
        	free(env[i]);
		free(env);
        conn->state = ON_CGI;
			
		return ;
    }

	int cgi_input[2];
	int cgi_output[2];

	if(pipe(cgi_input) < 0 || pipe(cgi_output) < 0)
	{
		conn->state = SEND_ERROR;
		conn->errorCode = INTERNAL_ERROR;
	
		free(env);

		return;
	}

	pid_t pid = fork();
	if(pid < 0)	
	{
		conn->state = SEND_ERROR;
		conn->errorCode = INTERNAL_ERROR;
	
		free(env);

		return ;
	}
	
	if(pid == 0)
	{
		close(cgi_input[1]);	
		close(cgi_output[0]);

		dup2(cgi_input[0], STDIN_FILENO);
		dup2(cgi_output[1], STDOUT_FILENO);

		execve(request->path, args, env);

		exit(0);
	}
	
	close(cgi_input[0]);
	close(cgi_output[1]);

	if(strcasecmp(request->method, "POST") == 0)
		write(cgi_input[1], request->query, strlen(request->query));

	conn->cevent = newBufferEvent(server->server->loop, cgi_output[0],
								onCgi, NULL, conn);
	enableRead(conn->cevent);

	close(cgi_input[1]);
	free(env);

	conn->state = ON_CGI;
}

void onCgi(struct BufferEvent* cevent, void* arg)
{
	assert(cevent != NULL);
	
	struct Buffer* input = cevent->input;

	struct httpConnection* conn = (struct httpConnection*)arg;
	struct Buffer* output = conn->bevent->output;
	bufferPrintf(output, "%s 200 OK\r\n", HTTP_VERSION);
	bufferPrintf(output, "Content-length: %d\r\n", 
						input->windex- input->rindex);
	bufferAddStr(output, input->buf, input->windex - input->rindex);
	freeBufferEvent(cevent);

	conn->state = SEND_RESPONSE;
		
	onRequest(conn->bevent, NULL);	
}

void sendResponse(struct httpConnection* conn)
{
	assert(conn != NULL);

#ifdef DEBUG
	printf("send response.\n");
#endif

	struct BufferEvent* bevent = conn->bevent;
	int sockfd = bevent->event->fd;
	assert(sockfd >= 0);
	
	struct Buffer* output = bevent->output;
	
	bufferWrite(output, sockfd);
	
	if(output->rindex < output->windex)
	{
		enableWrite(bevent);
	
		return;
	}

	conn->state = DISCONNECTED;
}

void sendError(struct httpConnection* conn)
{
	assert(conn != NULL);

	int errorCode = conn->errorCode;

#ifdef DEBUG
	printf("send error: %s\n", errorCodes[conn->errorCode]);
#endif
	struct Buffer* output = conn->bevent->output;
	assert(output != NULL);
	
	switch(errorCode)
	{
		case BAD_REQUEST:
			bufferPrintf(output, "%s 400 Bad Request\n", HTTP_VERSION);
			bufferPrintf(output, "Content-length: %u\r\n", 100);
			bufferPrintf(output, "Content-type: text/html\r\n");
			addGeneralHeader(conn);
			bufferPrintf(output, 
				"<html>\n"
				"<head>\n"
				"<title>Bad Request</title>\n"
				"</head>\n"
				"<body>\n"
				"Bad file name.\n"
				"</body>\n"
				"</html>\n");
			break;
		
		case NOT_FOUND:
			bufferPrintf(output, "%s 404 Not Found\n", HTTP_VERSION);
			bufferPrintf(output, "Content-length: %u\r\n", 100);
			bufferPrintf(output, "Content-type: text/html\r\n");
			addGeneralHeader(conn);
			bufferPrintf(output, 
				"<html>\n"
				"<head>\n"
				"<title>Not Found</title>\n"
				"</head>\n"
				"<body>\n"
				"File is not found.\n"
				"</body>\n"
				"</html>\n");
			break;

		case NOT_IMPLEMENTED:
			bufferPrintf(output, "%s 501 Not Implemented\n", HTTP_VERSION);
			bufferPrintf(output, "Content-length: %u\r\n", 100);
			bufferPrintf(output, "Content-type: text/html\r\n");
			addGeneralHeader(conn);
			bufferPrintf(output, 
				"<html>\n"
				"<head>\n"
				"<title>Not Found</title>\n"
				"</head>\n"
				"<body>\n"
				"That method is not implemented.\n"
				"</body>\n"
				"</html>\n");

			break;

		case FORBIDDEN:
			bufferPrintf(output, "%s 403 Forbidden\n", HTTP_VERSION);
			bufferPrintf(output, "Content-length: %u\r\n", 100);
			bufferPrintf(output, "Content-type: text/html\r\n");
			addGeneralHeader(conn);
			bufferPrintf(output, 
				"<html>\n"
				"<head>\n"
				"<title>Forbidden</title>\n"
				"</head>\n"
				"<body>\n"
				"File is protected.\n"
				"</body>\n"
				"</html>\n");
			break;

		case INTERNAL_ERROR:
			bufferPrintf(output, "%s 500 Internal Error\n", HTTP_VERSION);
			bufferPrintf(output, "Content-length: %u\r\n", 100);
			bufferPrintf(output, "Content-type: text/html\r\n");
			addGeneralHeader(conn);
			bufferPrintf(output, 
				"<html>\n"
				"<head>\n"
				"<title>Internal Error</title>\n"
				"</head>\n"
				"<body>\n"
				"Something unexpected went wrong.\n"
				"</body>\n"
				"</html>\n");

			break;
		
		default:
			break;
	}

	sendResponse(conn);
}

char** makeEnv(struct httpConnection* conn)
{
	assert(server != NULL);

	struct Setting* setting = server->setting;
	assert(setting != NULL);

	assert(conn != NULL);

	struct httpRequest* request = conn->request;
	assert(request != NULL);	

	char** env = (char**)malloc(50 * sizeof(char*));
	assert(env != NULL);

	int envn = 0;
	char buf[1024];
	
	snprintf(buf, sizeof(buf) - 1, "PATH=%s", CGI_PATH);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "LD_LIBRARY_PATH=%s", CGI_LD_LIBRARY_PATH);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "SERVER_SOFTWARE=%s", SERVER_SOFTWARE);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "SERVER_NAME=%s", SERVER_NAME);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "GATEWAY_INTERFACE=%s", GATEWAY_INTERFACE);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "SERVER_PROTOCOL=%s", SERVER_PROTOCOL);
	env[envn++] = strdup(buf);
	
	snprintf(buf, sizeof(buf) - 1, "SERVER_PORT=%d", setting->listen);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "REQUEST_METHOD=%s", request->method);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "SCRIPT_NAME=%s", request->url);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "PATH_INFO=/%s", request->url);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "QUERY_STRING=%s", request->query);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "REMOTE_ADDR=%s", conn->remote);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "HTTP_REFERER=%s", request->referrer);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "HTTP_REFERRER=%s", request->referrer);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "HTTP_USER_AGENT=%s", request->userAgent);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "HTTP_COOKIE=%s", request->cookie);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "HTTP_HOST=%s", request->host);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "CONTENT_TYPE=%s", request->contentType);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "CONTENT_LENGTH=%ld", request->contentLength);
	env[envn++] = strdup(buf);

	snprintf(buf, sizeof(buf) - 1, "AUTH_TYPE=%s", AUTH_TYPE);
	env[envn++] = strdup(buf);

	env[envn] = NULL;

	return env;
}

struct httpConnection* newConnection(int sockfd, struct BufferEvent* bevent, 								struct httpServer* server, struct EventLoop* loop)
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
	conn->state = PARSE_FIRSTLINE;
	conn->request = (struct httpRequest*)malloc
						(sizeof(struct httpRequest));
	memset(conn->request, 0, sizeof(struct httpRequest));
	assert(conn->request != NULL);

	getPeerAddr(sockfd, conn->remote, sizeof(conn->remote));

	return conn;
}

void freeConnection(struct httpConnection* conn)
{
	assert(conn != NULL);
	
	int sockfd = conn->sockfd;
	server->connMap[sockfd] = NULL;
	if(conn->request->path)
		free(conn->request->path);
	free(conn->request);
	freeBufferEvent(conn->bevent);
	free(conn);
}

void newHttpServer(int argc, char* argv[])
{
	server = (struct httpServer*)malloc(sizeof(struct httpServer));
	assert(server != NULL);

	server->setting = parseOpt(argc, argv);
	assert(server->setting != NULL);

	logOpen(server->setting->logFile, server->setting->logLevel);

	struct EventLoop* loop = (struct EventLoop*)malloc
								(sizeof(struct EventLoop));
	assert(loop != NULL);
	eventLoopInit(loop);	

	server->server = newServer(loop, server->setting->listen,
								server->setting->serverName, 
								strlen(server->setting->serverName));
	assert(server->server != NULL);
	server->acceptor = server->server->sockfd;
	
	setThreadNumber(server->server, server->setting->nthreads);
	
	server->connMap = (struct httpConnection**)malloc
						(CONN_INIT_SIZE * sizeof(struct httpConnection*));
	assert(server->connMap != NULL);
	server->mapSize = CONN_INIT_SIZE;

	setAcceptCb(server->server, onConnection);
	setReadCb(server->server, onRequest);
	setWriteCb(server->server, onResponse);	
}

void startServer()
{
	struct Setting* setting = server->setting;

	if(setting->daemon)
	{
#ifdef DEBUG
	printf("run as a daemon!\n");
#endif
		daemonize();
		singleRun(server->setting->pidFile);
	}

	if(getuid() == 0)
	{
		struct passwd* pwd;

		pwd = getpwnam("nobody");
		if(pwd == NULL)
		{
			LogError("user %s is not exist.\n", "nobody");
			exit(0);
		}

		int uid = pwd->pw_uid;
		int gid = pwd->pw_gid;

		char logFile[256];
		char pidFile[256];
		snprintf(logFile, sizeof(logFile), "%s%s", 
					setting->root, setting->logFile);
		snprintf(pidFile, sizeof(pidFile), "%s%s", 
					setting->root, setting->pidFile);
		if(chown(logFile, uid, gid) < 0)
		{
			extern int errno;
			LogError("logfile chown() error: %s\n", strerror(errno));
		}
		
		if(chown(pidFile, uid, gid) < 0)
		{
			extern int errno;
			LogError("logfile chown() error: %s\n", strerror(errno));
		}

/*		if(chroot(setting->root) < 0)
		{
			LogError("chroot() error: %s\n", strerror(errno));
			exit(0);
		}*/
	
		LogDebug("now root = %s\n", setting->root);
		
		/*if(setuid(uid) < 0)
		{
			LogError("setuid() error: %s\n", strerror(errno));
			exit(0);
		}*/
		
		/*if(setgid(gid) < 0)
		{
			LogError("setgid() error: %s\n", strerror(errno));

			exit(0);
		}*/
	}

	start(server->server);		
}

void stopServer(struct httpServer* server)
{
	
}

void daemonize()
{
	struct rlimit rl;
	pid_t pid;

	umask(0);

	if(getrlimit(RLIMIT_NOFILE, &rl) < 0)
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

	int fd0, fd1, fd2;
	
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

	/*if(lockfile(fd) < 0)
	{
		if(errno == EAGAIN)
		{
			close(fd);
			fprintf(stderr, "webServer is already running!\n");
			exit(0);
		}
		
		perror("can't lock file error:");
		exit(0);
	}*/
	
	ftruncate(fd, 0);
	
	char buf[16];
	snprintf(buf, sizeof(buf), "%ld", (long)getpid());
	write(fd, buf, strlen(buf));
}
