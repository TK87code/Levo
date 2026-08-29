#include "levo.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

bool lev_str_match(const char *pattern, const char *str)
{
	if (*pattern == '\0') {
		if (*str == '\0') {
			return true;
		} else {
			return false;
		}
	}

	if (*pattern == '*') {
		if (lev_str_match(pattern + 1, str))
			return true;
		else if (*str != '\0')
			return lev_str_match(pattern, str + 1);
	}

	if (*pattern == *str || (*pattern == '?' && *str != '\0'))
		return lev_str_match(pattern + 1, str + 1);	
	else
		return false;
}

void lev_str_tolower(char *text) 
{
	for (size_t i = 0; text[i] != '\0'; i++) {
		unsigned char c = (unsigned char)text[i];
		if (isalpha(c))
			text[i] = (char)tolower(c);
	}
}

void lev_str_alpha_only(char *text)
{
	int head = 0;
	int tail = 0;
	while (text[head] != '\0') {
		unsigned char c = (unsigned char)text[head];
		if (isalpha(c)) {
			text[tail] = text[head];
			tail++;
		}
		head++;
	}
	text[tail] = '\0';
}

const char *lev_cli_parse(int argc, char *argv[], struct lev_cli_option opts[], 
			  size_t num_opts, const char *rests[], int *rests_count)
{
	if (!argv || argc < 1 || !rests || !rests_count)
		return "";

	*rests_count = 0;

	// Reset option result states
	if (opts) {
		for (size_t j = 0; j < num_opts; j++) {
			opts[j].is_set = false;
			opts[j].value = NULL;
		}
	}

	bool force_quit_parse_opts = false;

	for (int i = 1; i < argc; i++) {
		if (!force_quit_parse_opts && strcmp(argv[i], "--") == 0) {
			force_quit_parse_opts = true;
			continue;
		}

		// Positional arguments (or following "--", or standalone "-")
		if (force_quit_parse_opts || argv[i][0] != '-' || argv[i][1] == '\0') {
			rests[*rests_count] = argv[i];
			(*rests_count)++;
			continue;
		}

		bool is_known = false;

		if (opts) {
			for (size_t j = 0; j < num_opts; j++) {
				if (opts[j].name && strcmp(opts[j].name, argv[i]) == 0) {
					is_known = true;
					opts[j].is_set = true;

					if (opts[j].has_value) {
						if (i + 1 < argc) {
							i++;
							opts[j].value = argv[i];
						} else {
							// Missing required value argument
							return argv[i];
						}
					}
					break;
				}
			}
		}

		if (!is_known)
			return argv[i];
	}

	rests[*rests_count] = NULL;

	return NULL;
}

#if defined(_WIN32) || defined(_WIN64)
	#include <direct.h>
	#define _lev_get_current_dir _getcwd
#else
	#include <unistd.h>
	#define _lev_get_current_dir getcwd
#endif //defined(_WIN32) || defined(_WIN64)

char *lev_os_getcwd(char* out_buf, size_t size)
{
	return _lev_get_current_dir(out_buf, size);
}

#if defined(_WIN32) || defined(_WIN64)
	#include <windows.h>
	int lev_term_enable_ansiesc(void)
	{
		HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (handle == INVALID_HANDLE_VALUE) 
			return -1;

		DWORD mode = 0; 
		if (!GetConsoleMode(handle, &mode))
			return -2;

		// ENABLE_VIRTUAL_TERMINAL_PROCESSING (0x0004)
		mode |= 0x0004;
		SetConsoleMode(handle, mode);

		return 0;
	}

#else
	int lev_term_enable_ansiesc(void) 
	{
		return 0;	
	}
#endif //defined(_WIN32) || defined(_WIN64)

void lev_printf_color(int color, const char *fmt, ...)
{
	if (color < 0)
		color = 0;
	if (color > 255)
		color = 255;

	printf("\x1b[38;5;%dm", color);

	char buf[1024] = {0};
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	printf("%s",buf);
	printf("\x1b[0m");
	fflush(stdout);
}

int lev_rand(uint32_t *seed, int min, int max)
{
	if (!seed)
		return min;

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

	uint64_t range = (uint64_t)((int64_t)max - (int64_t)min + 1);
	uint64_t offset = (uint64_t)x % range;

	return (int)((int64_t)min + (int64_t)offset);
}

int lev_read_stdin(char *out_buf, size_t buf_size, int terminator)
{
	if (!out_buf || buf_size == 0)
		return -1;

	size_t i = 0;
	int c = EOF;

	while (i + 1 < buf_size) {
		c = getchar();

		if (c == EOF) {
			if (i == 0) {
				out_buf[0] = '\0';
				return -3;
			}
			break;
		}

		if (c == terminator)
			break;

		out_buf[i++] = (char)c;
	}
	out_buf[i] = '\0';

	if (i + 1 == buf_size && c != terminator && c != EOF)
		return -2;

	return (int)i;
}

