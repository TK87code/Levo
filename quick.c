#include "mtb.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	char buffer[512];
	printf("Please input something:\n>> ");
	mtb_read_stdin(buffer, 512, '\n');
	printf("%s\n", buffer);
	return 0;
}
