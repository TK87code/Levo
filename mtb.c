#include "mtb.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// =============
// Consoles
// =============
int mtb_read_stdin(char *out_buf, size_t buf_size, int terminator)
{
	if (!out_buf || buf_size == 0)
		return -1;

	int c = 0;
	size_t i;
	
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

const char *mtb_parse_clargs(int argc, char *argv[], struct mtb_cl_option opts[], size_t num_opts, const char *rests[], int *rests_count)
{
	*rests_count = 0;
	bool force_quit_parse_opts = false;

	for (int i = 1; i < argc; i++) {
		if (!force_quit_parse_opts && strcmp(argv[i], "--") == 0) {
			force_quit_parse_opts = true;
			continue;		
		}

		// Pickup non-optional arguments including single '-'
		if (force_quit_parse_opts || argv[i][0] != '-' || argv[i][1] == '\0') {
			rests[*rests_count] = argv[i]; 
			*rests_count += 1;
			continue;
		}

		bool is_known = false;

		for (size_t j = 0; j < num_opts; j++) {
			if ((strcmp(opts[j].name, argv[i])) == 0) {
				is_known = true;	
				opts[j].is_set = true;
				
				if (opts[j].has_value) {
					if (i + 1 < argc) { 
						i += 1;
						opts[j].value = argv[i];
					} else {
						// No value where it should be
						return argv[i];
					}
				}

				break;
			}
		}

		if (is_known == false)
			return argv[i];
	}

	rests[*rests_count] = NULL;

	return NULL;
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
