#include "mtb.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#define STR_MAX 512

int main(int argc, char **argv)
{
	int key = 0;
	int mode = 0;
	if (argc != 3) {
		printf("Usage:caesar_cipher <mode(-e or -d)> <Number of character shift>\n");
		return 1;
	}

	mode = argv[1][1];
	if (mode != 'e' && mode != 'd') {
		printf("Invalid mode\n");
		return 1;
	}

	key = atoi(argv[2]);
	if (key < 0 || key > 25) {
		printf("Invalid key\n");
		return 1;
	}
		
	int ch = 0;
	int buffer[STR_MAX] = {0};
	int count = 0;

	while ((ch = getchar()) != EOF && count < STR_MAX){
		if (ch == '\n')
			break;
		buffer[count] = ch;
		count++;		
	}
	
	char out_str[STR_MAX] = {0};

	for (int i = 0; i < count; i++) {
		int x = buffer[i];

		if (isspace(x)) {
			out_str[i] = ' ';
			continue;
		}

		if (isupper(x)){
			if (mode == 'e')
				out_str[i] = ((x - 65 + key) % 26) + 65;
			else if (mode == 'd')
				out_str[i] = ((x - 65 - key + 26) % 26) + 65;	
		} else if (islower(x)) {
			if (mode == 'e') 
				out_str[i] = ((x - 97 + key) % 26) + 97;
			else if (mode == 'd') 
				out_str[i] = ((x - 97 - key + 26) % 26) + 97;		
		}
	}

	out_str[count] = '\0';

	printf("%s\n", out_str);
	return 0;
}
