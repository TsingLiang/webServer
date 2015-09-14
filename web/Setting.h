#ifndef SETTING_H
#define SETTING_H

#include <stdbool.h>

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
};

struct Setting* parseOpt(int argc, char* argv[]);
struct Setting* parseConf(const char* file);

#endif
