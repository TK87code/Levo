#include "levo.h"
#include <string.h>
#include <stdio.h>
 
static bool is_palindrome_recursive(const char *start, const char *end)
{
	if (start >= end)
		return true;
	else if (*start != *end)
		return false;
	else
		return is_palindrome_recursive(start + 1, end - 1);
}

bool is_palindrome(const char *text) 
{
	const char *start = text;
	const char *end = &text[strlen(text) - 1];

	return is_palindrome_recursive(start, end);
}

int main(void)
{
	printf("Enter a text to check if it's a palindrome.\n>> ");	
	char buf[512] = {0};
	
	lev_read_stdin(buf, sizeof(buf), '\n');

	lev_str_tolower(buf);
	lev_str_alpha_only(buf);

	if (is_palindrome(buf))
		printf("It's a palindrome indeed!\n");
	else
		printf("It's not a palindrome.\n");
	
	return 0;
}
