#include "levo.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv)
{
	int max = 0;

	if (argc != 2) {
		printf("Usage: dice <6> or <10> etc..\n");
		return 1;
	} else {
		max = atoi(argv[1]);
	}

	if (max <= 0)
		return 1;

	uint32_t seed = (uint32_t)time(NULL);
	
	int n = lev_rand(&seed, 1, max);
	printf("%d\n", n);

	return 0;
}
