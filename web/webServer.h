#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <time.h>

#define DEBUG

enum ConnectionState
{
	DISCONNECTED,
	CONNECTED,

	PARSE_FIRSTLINE,
	PARSE_HEADER,

	PRE_RESPONSE,
	DO_FILE,
	DO_DIR,
	DO_CGI,
	ON_CGI,
	
	SEND_RESPONSE,
	SEND_ERROR
};

enum StatusCode
{
	HTTP_OK,
	BAD_REQUEST,
	NOT_FOUND,
	NOT_IMPLEMENTED,
	FORBIDDEN,
	INTERNAL_ERROR		
};

struct httpRequest
{
//first line
	char* method;
	char* url;
	char* path;
	char* query;
	char* protocol;
	char major;
	char minor;
//header
	char* authorization;
	long  contentLength;
	char* contentType;
	char* cookie;
	char* host;
	char* modified;
	char* referrer;
	char* userAgent;
	char* accept;
	char* acceptLanguage;
	char* acceptEncoding;
	char* acceptCharset;
	char* keepAlive;
	char* connection;
};

struct httpServer;

struct httpConnection
{
	int sockfd;
	struct BufferEvent* bevent;
	struct BufferEvent* cevent;
	time_t timeout;
	struct EventLoop* loop;
	struct httpServer* server;
	int state;

	struct httpRequest* request;
	int errorCode;
	
	char remote[25];	
};

struct ConcurrentHashMap;

struct httpServer
{
	int acceptor;
	char hostport[25];
	short port;
	struct Server* server;
	struct httpConnection** connMap;
	int mapSize;

	struct ConcurrentHashMap* map;
	
	struct Setting* setting;
};

struct httpConnection* newConnection(int sockfd, struct BufferEvent* bevent, 								struct httpServer* server, struct EventLoop* loop);
void freeConnection(struct httpConnection* conn);

void newHttpServer(int argc, char* argv[]);
void startServer();
void stopServer();

#endif
