#include "sne.h"
#include <stdio.h>
#include <stdlib.h>
/**
 
static bool is_palindrome_recursive(const char *start, const char *end)
{
	if (start >= end)
		return true;
	else if (*start != *end)
		return false;
	else
		return is_palindrome_recursive(start + 1, end - 1);
}

//TODO Complete palindrome app. need sanitize function. (to-lower, and remove non-alphabet character)
//isalpha() & tolower() <ctypel.h>
bool is_palindrome(const char *text) {
	const char *start = text;
	const char *end = &text[strlen(text) - 1];

	return is_palindrome_recursive(start, end);
}
*/

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
			if (sne_str_match("*TODO*", line))
				printf("%s:%d: %s", argv[i], line_count, line);
			line_count++;
		}
		
		fclose(fp);
	}
	
	return EXIT_SUCCESS;
}
