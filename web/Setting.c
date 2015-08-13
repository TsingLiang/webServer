#include "Setting.h"
#include "cJSON.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>


static const char VERSION[] = "0.1";

static const int defaultListenPort = 80;
static const int defaultThreads = 4;

static void usage()
{
	printf("webServer. Configure it by pidfile or options.\n");
	printf(	"-h		help\n"
			"-v		version\n"
			"-c		configure file\n"

			"follows are specified options\n"
			"-p		pidfile\n"
			"-d		run as a daemon server\n"
			"-u		user\n"
			"-r		work root\n"
			"-w		www document directory\n"
			"-l		listen port\n"
			"-t		number of thread\n"
			"-f		path of logfile\n"
			);
}

struct Setting* parseOpt(int argc, char* argv[])
{
	char c;
	char* conf = NULL;
	char* pidfile = NULL;
	char* logfile = NULL;
	bool  daemon = false;
	char* user = NULL;
	char* root = NULL;
	char* document = NULL;
	int listen = 80;
	int nthreads = 4;

	while( (c = getopt(argc, argv, 
			"h"		//help
			"v"		//version
			"c:"		//configure file
			"p:"		//pidfile
			"d"		//run as a daemon
			"u:"		//user
			"r:"		//work root
			"w:"		//www document directory
			"l:"		//listen port
			"t:"		//number of thread
			"f:"		//path of logfile
			)) != -1)
	{
		switch(c)
		{
			case 'h':
				usage();
				exit(0);
			
			case 'v':
				printf("webServer version = %s\n", VERSION);
				exit(0);

			case 'c':
				conf = strdup(optarg);
				break;
					
			case 'p':
				pidfile = strdup(optarg);
				break;

			case 'd':
				daemon = true;
				break;

			case 'u':
				user = strdup(optarg);
				break;
			
			case 'r':
				root = strdup(optarg);
				break;
			
			case 'w':
				document = strdup(optarg);
				break;			

			case 't':
				nthreads = atoi(optarg);
				break;
			
			case 'f':
				logfile = strdup(optarg);
				break;

			default:
				printf("unkown option %s.\n", optarg);
				exit(0);
		}
	}
	
	struct Setting* setting;
	
	if(conf)
	{
		setting = parseConf(conf);
	}
	else
	{
		setting = (struct Setting*)malloc(sizeof(struct Setting));
		assert(setting != NULL);

		if(root)
			setting->root = root;
		else
			setting->root = "/usr/local/webServer";
		
		if(document)
			setting->document = document;
		else
			setting->document = "www";
		
		if(pidfile)
			setting->pidFile = pidfile;
		else
			setting->pidFile = "webServer.pid";

		if(logfile)
			setting->logFile = logfile;
		else
			setting->logFile = "webServer.log";
	
		setting->listen = defaultListenPort;
		setting->nthreads = defaultThreads;
		setting->daemon = daemon;	
	}
	
	return setting;
}
struct Setting* parseConf(const char* conf)
{
	int fd = open(conf, O_RDONLY);
	if(fd < 0)
	{
		perror("open() error:");
		exit(0);
	}
	struct stat st;
	if(fstat(fd, &st) < 0)
	{
		perror("fstat() error: ");
		exit(0);
	}
	
	char* text = (char*)malloc(st.st_size + 1);
	assert(text != NULL);
	int n = read(fd, text, st.st_size);
	if(n != st.st_size)
	{
		printf("read conf file error!\n");
		exit(0);
	}
	text[n] = '\0';
	
	cJSON* webServer = cJSON_Parse(text);
	if(webServer == NULL)
	{
		printf("parse conf error: %s\n", cJSON_GetErrorPtr());
		exit(0);
	}
	printf("webServer: %s\n", cJSON_Print(webServer));
	
	struct Setting* setting = (struct Setting*)malloc(sizeof(struct Setting));
	assert(setting != NULL);

	cJSON* directory = cJSON_GetObjectItem(webServer, "directory");
	if(directory)
	{
		cJSON* root = cJSON_GetObjectItem(directory, "root");
		if(root)
			setting->root = strdup(root->valuestring);
		else
			setting->root = "/usr/local/webServer";
		
		cJSON* document =  cJSON_GetObjectItem(directory, "document");
		if(document)
			setting->document = strdup(document->valuestring);
		else
			setting->document = "www";
	}
	else
	{
		setting->root = "/usr/local/webServer";
		setting->document = "html";
	}

	cJSON* file = cJSON_GetObjectItem(webServer, "file");
	if(file)
	{
		cJSON* pidFile = cJSON_GetObjectItem(file, "pidfile");
		if(pidFile)
			setting->pidFile = strdup(pidFile->valuestring);
		else
			setting->pidFile = "webServer.pid";

		cJSON* logFile = cJSON_GetObjectItem(file, "logfile");
		if(logFile)
			setting->logFile = strdup(logFile->valuestring);
		else
			setting->logFile = "webServer.log";
	}
	else
	{
		setting->pidFile = "webServer.pid";
		setting->logFile = "webServer.log";
	}

	cJSON* server = cJSON_GetObjectItem(webServer, "server");
	if(server)
	{
		cJSON* listen = cJSON_GetObjectItem(server, "listen");
		if(listen)
			setting->listen = listen->valueint;
		else
			setting->listen = defaultListenPort;
	
		cJSON* nthreads = cJSON_GetObjectItem(server, "nthreads");
		if(nthreads)
			setting->nthreads = nthreads->valueint;
		else
			setting->nthreads = defaultThreads;

		cJSON* daemon = cJSON_GetObjectItem(server, "daemon");
		if(daemon)
			setting->daemon = daemon->type == 1 ? true : false;
		else
			setting->daemon = false;
	}
	else
	{
		setting->listen = 80;
		setting->nthreads = 4;
		setting->daemon = true;
	}	

	close(fd);
	cJSON_Delete(webServer);

	return setting;
}	

