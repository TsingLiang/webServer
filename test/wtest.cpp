#include "Writer.h"
#include <string.h>

int main()
{
	Writer* writer = new BufferedWriter("./test.txt", true);
	char str[] = "write test.";

	writer->write(str, 0, strlen(str));
	writer->flush();
	writer->close();
	delete writer;

	return 0;
}
