// peek.c - v1.0 
// About: Simple CLI C file viewer with syntax highlight and line numbers.
// Last updated: 2026/8/30 

#include "levo.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_WORD_BYTE 512

#define DARK_YELLOW 166
#define YELLOW 3 
#define CYAN 6
#define PINK 132
#define RED 1
#define GREY 243
#define GREEN 156
#define DARK_GREEN 29

static const char *dark_yellows [] = {"const", "struct", "extern", "inline", "static"};

static const char *yellows [] = {"int", "float", "void", "size_t", "bool", "char", "unsigned", "signed", "short", "long", "double", "int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t", "int64_t", "uint64_t", "wchar_t"};

static const char *cyans [] = {"#include", "#define", "#ifdef", "#endif", "#ifndef", "#if", "#else"};

static const char *pinks [] = {"sizeof"};

static const char *reds [] = {"for", "if", "return", "while", "else"};

static size_t yellow_num = sizeof(yellows) / sizeof(yellows[0]);
static size_t dark_yellow_num = sizeof(dark_yellows) / sizeof(dark_yellows[0]);
static size_t cyan_num = sizeof(cyans) / sizeof(cyans[0]);
static size_t pink_num = sizeof(pinks) / sizeof(pinks[0]);
static size_t red_num = sizeof(reds) / sizeof(reds[0]);

static bool detect_color(const char *word, const char *colors[], size_t num)
{
	for (size_t i = 0; i < num; i++) {
		if (strcmp(word, colors[i]) == 0)
				return true;
	}

	return false;
}

int main(int argc, char *argv[])
{
	lev_term_enable_ansiesc();

	if (argc != 2) {
		printf("Usage: peek <file>\n");
		return 1;
	}

	FILE *fp = fopen(argv[1], "r");
	if (!fp) {
		printf("peek: %s: No such file.\n", argv[1]);
		return 1;
	}
	
	char word[MAX_WORD_BYTE] = {0};
	size_t w_len = 0;
	int c;
	bool is_comment = false;
	bool is_double_quoted = false;
	bool is_single_quoted = false;
	bool is_quote_escaped = false;
	int line_count = 1;

	int prev_c = 0;
	bool is_multiline_comment = false;
	
	printf("%4d| ", line_count);

	while ((c = fgetc(fp)) != EOF) {
		if (prev_c == '\n') {
			line_count += 1;
			printf("%4d| ", line_count);
		}

		if (is_comment) {
			lev_printf_color(GREY, "%c", c);
			if (c == '\n')
				is_comment = false;
			prev_c = c;
			continue;
		}

		if (is_multiline_comment) {
			lev_printf_color(GREY, "%c", c);
			if (prev_c == '*' && c == '/') 
				is_multiline_comment = false;
			prev_c = c;
			continue;
		}

		if (is_double_quoted) {
			lev_printf_color(DARK_GREEN, "%c", c);
			if (is_quote_escaped) {
				is_quote_escaped = false;
			} else if (c == '\\') {
				is_quote_escaped = true;
			} else if (c == '"') {
				is_double_quoted = false;
			}
			prev_c = c;
			continue;
		}

		if (is_single_quoted) {
			lev_printf_color(GREEN, "%c", c);
			if (is_quote_escaped) {
				is_quote_escaped = false;
			} else if (c == '\\') {
				is_quote_escaped = true;
			} else if (c == '\'') {
				is_single_quoted = false;
			}
			prev_c = c;
			continue;
		}

		if (w_len < MAX_WORD_BYTE - 1 && (isalnum(c) || c == '_' || c == '#' || c == '<' || c == '>' || c == '.')) {
			word[w_len] = c;
			w_len++;
		} else if (c == '"') {
			is_double_quoted = true;	
			lev_printf_color(DARK_GREEN, "%c", c);
		} else if (c == '\'') {
			is_single_quoted = true;
			lev_printf_color(GREEN, "%c", c);
		} else { 
			word[w_len] = '\0';
			
			if (lev_str_match("<*>", word)) {
				lev_printf_color(DARK_GREEN, "%s", word);
			} else {
				if (detect_color(word, yellows, yellow_num)) {
					lev_printf_color(YELLOW, "%s", word);
				} else if (detect_color(word, dark_yellows, dark_yellow_num)) {
					lev_printf_color(DARK_YELLOW, "%s", word);
				} else if (detect_color(word, cyans, cyan_num)) {
					lev_printf_color(CYAN, "%s", word);
				} else if (detect_color(word, pinks, pink_num)) {
					lev_printf_color(PINK, "%s", word);
				} else if (detect_color(word, reds, red_num)) {
					lev_printf_color(RED, "%s", word);
				} else if(isdigit((unsigned char)word[0])) {
					lev_printf_color(PINK, "%s", word);	
				} else {
					printf("%s", word);
				}
			}

			w_len = 0;

			if (c == '/') {
				int next_c = fgetc(fp);

				if (next_c == '*') {
					is_multiline_comment = true;
					lev_printf_color(GREY, "/*");
					prev_c = '*';
					continue;
				} else if (next_c == '/') {
					is_comment = true;
					lev_printf_color(GREY, "//");
					prev_c = '/';
					continue;
				} else {
					ungetc(next_c, fp);
					putc(c, stdout);
				}
			} else {
				putc(c, stdout);
			}
		}
		prev_c = c;
	}
	
	fclose(fp);
	return 0;
}
