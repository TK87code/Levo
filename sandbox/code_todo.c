#include "levo.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(_WIN64)
int _dowildcard = -1; 
#endif
	
int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: code_todo <filename>\n");
		return EXIT_FAILURE;
	}
	
	for (int i = 1; i < argc; i++) {
		char line[1024] = {0};
		FILE *fp = fopen(argv[i], "r");
		if (!fp) {
			continue;
		}

		int line_count = 1;
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (lev_str_match("*TODO*", line)) {
				lev_printf_color(172, "%s:%d: ", argv[i], line_count);
				printf("%s", line);
			}
			line_count++;
		}
		
		fclose(fp);
	}
	
	return EXIT_SUCCESS;
}
