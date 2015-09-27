#ifndef SETTING_H
#define SETTING_H

#include <stdbool.h>

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

	char* serverName;		
	int  listen;
	int  nthreads;
	bool daemon;

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
