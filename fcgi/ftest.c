#include "fcgi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	int count = 0;

	while( fcgi_accept(argc, argv) >= 0)
	{
		printf("Contenttype: text/html\n\n");

		printf("HTTP_USER_AGENT = %s\n", getenv("HTTP_USER_AGENT"));
		printf("SERVER_SOFTWARE = %s\n", getenv("SERVER_SOFTWARE"));
		printf("QUERY_STRING = %s\n", getenv("QUERY_STRING"));
	
		printf("count = %d\n", ++count);
	}

	return 0;
}
