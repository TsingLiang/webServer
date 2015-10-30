#include "Reader.h"
#include <stdio.h>

int main()
{
	Reader* reader = new BufferedReader("./test.txt");
	
	int n;
	char buf[BUFSIZ];
	while( (n = reader->read(buf, 0, BUFSIZ)) != -1 )
	{
		buf[n] = '\0';
		printf("%s", buf);
	}

	reader->close();
	delete reader;

	return 0;
}
