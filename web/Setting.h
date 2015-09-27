#ifndef SETTING_H
#define SETTING_H

#include <stdbool.h>
#include <time.h>

struct fcgi_location
{
	char* file;
	char* ipport;
};

struct Setting
{
	char* root;
	char* document;

	char* pidFile;
	char* logFile;
	int   logLevel;

	//server
	char* serverName;		
	int  listen;
	int  nthreads;
	bool daemon;

	//cache
	bool usecache;
	int tablesize;
	time_t html;//msecond
	time_t cgi;

	//fcgi
	bool  usefcgi;
	char* dir;
	char* index;
	int   size;
	struct fcgi_location location[0];
};

struct Setting* parseOpt(int argc, char* argv[]);
struct Setting* parseConf(const char* file);

#endif
