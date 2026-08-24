#include "mtb.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define OPT_F 0
#define OPT_A 1
int main(int argc, char *argv[])
{
	struct mtb_cl_option opts[] = {
		[OPT_F] =	{"-f", NULL, true, false},
		[OPT_A] =	{"-a", NULL, false, false},
	};

	const char *rests[argc];
	int rests_count = 0;

	const char *error_opt = mtb_parse_clargs(argc, argv, opts, 2, rests, &rests_count);
	if (error_opt != NULL) {
		printf("Unknown option '%s' passed\n", error_opt);
		printf("Use '-h' to see available options.\n");
		return 1;
	}

	if (opts[OPT_F].is_set) {
		if (opts[OPT_F].value == NULL)
			printf("-f need value afterwards\n");
		else
			printf("-f is set value: %s\n", opts[OPT_F].value);
	}

	if (opts[OPT_A].is_set)
		printf("-a is set\n");

	for (int i = 0; i < rests_count; i++) {
		printf("Non-option args #%d : %s\n", i, rests[i]);
	}

	return 0;
}
