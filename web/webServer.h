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
	ADD_HEADER,
	DO_FILE,
	DO_DIR,
	DO_CGI,
	SEND_RESPONSE
};

struct httpRequest
{
//first line
	char* method;
	char* url;
	char* query;
	char* protocol;
	char major;
	char minor;
//header
	char* cookie;
	char* host;
	char* userAgent;
	char* accept;
	char* acceptLanguage;
	
};

struct httpServer;

struct httpConnection
{
	int sockfd;
	struct BufferEvent* bevent;
	time_t timeout;
	struct EventLoop* loop;
	struct httpServer* server;
	int state;
	struct httpRequest* request;
};

struct httpServer
{
	int acceptor;
	char hostport[25];
	struct Server* server;

	struct Setting* setting;
	struct Logger* logger;
};

struct httpConnection* newConnection(int sockfd, struct BufferEvent* bevent, 								struct httpServer* server, struct EventLoop* loop);
void freeConnection(struct httpConnection* conn);

struct httpServer* newServer();
void startServer(struct httpServer* server);
void stopServer(struct httpServer* server);

#endif
