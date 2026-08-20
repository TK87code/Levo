#include "mtb.h"

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
