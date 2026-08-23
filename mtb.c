#include "mtb.h"
#include <stdio.h>
// =============
// Consoles
// =============
int mtb_read_stdin(char *out_buf, size_t buf_size, int terminator)
{
	if (!out_buf || buf_size == 0)
		return -1;

	int c = 0;
	int i;
	
	for (i = 0; i < buf_size - 1; i++) {
		c = getchar();

		if (c == EOF) {
			if (i == 0)
				return -3;
			break;
		}

		if (c == terminator) {
			break;
		}

		out_buf[i] = (char)c;
	}
	out_buf[i] = '\0';

	if (i == buf_size - 1 && c != terminator && c != EOF)
		return -2;

	return i;
}

// =============
// Utils
// =============
int mtb_rand(uint32_t *seed, int min, int max)
{
	if (min > max) {
		int tmp = min;
		min = max;
		max = tmp;
	}

	uint32_t x = *seed;
	if (x == 0)
		x = 1;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

	*seed = x;	
        	
	return min + (x % (max - min + 1));
}
