#include "levo.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

enum lev_img_formats {
	LEV_IMGFMT_PGM_ASCII,
	LEV_IMGFMT_PGM_RAW,
};

static int _lev_img_read_magic(FILE *fp)
{
	// NOTE: This chunk should be changed when parsing other img formats
	//     which uses magic number of more than 8 bytes
	uint8_t magic[8];
	size_t bytes_read = fread(magic, 1, 8, fp); 

	if (bytes_read != 8) {
		return -1;
	}

	if (magic[0] == 'P' && magic[1] == '2') 
		return LEV_IMGFMT_PGM_ASCII;
	else if (magic[0] == 'P' && magic[1] == '5')
		return LEV_IMGFMT_PGM_RAW;
	else
		return LEV_IMG_ERR_HEADER;
}

static int _lev_img_skip_pgm(FILE *fp)
{
	int c;
	while(1) {
		c = fgetc(fp);

		if (c == EOF) {
			return -1;
		} else if (c == '#') {
			// Scan other than '\n' and dont store 
			if (fscanf(fp, "%*[^\n]") == EOF) 
				return -1;
		} else if (isspace(c)) {
			continue;
		} else {
			break;
		}
	}
	ungetc(c, fp);

	return 0;
}

static int _lev_img_info_pgm(FILE *fp, size_t *width, size_t *height, size_t *max_val)
{
	if (_lev_img_skip_pgm(fp) != 0 || fscanf(fp, "%zu", width) != 1)
		return -1;	
	if (_lev_img_skip_pgm(fp) != 0 || fscanf(fp, "%zu", height) != 1)
		return -1;	
	if (_lev_img_skip_pgm(fp) != 0 || fscanf(fp, "%zu", max_val) != 1)
		return -1;	

	(void)fgetc(fp); // dump the last spaces after max_val

	return 0;
}

static int _lev_img_skip_to_pixel(FILE *fp, size_t *max_val) 
{
	fseek(fp, 2, SEEK_SET);
	size_t dump;
	if (_lev_img_info_pgm(fp, &dump, &dump, max_val) < 0)
		return -1;

	return 0;
}

int lev_img_info(const char *path, size_t *out_width, size_t *out_height, size_t *out_bytes_per_pixel)
{
	if (!path) 
		return LEV_ERR_INVALID; 

	size_t dump;

	if (!out_width) 
		out_width = &dump;
	if (!out_height) 
		out_height = &dump;
	if (!out_bytes_per_pixel)
		out_bytes_per_pixel = &dump;

	int res = 0;

	FILE *fp = fopen(path, "rb");
	if (!fp) {
		res = LEV_ERR_FOPEN;
		goto cleanup;
	}

	int magic = _lev_img_read_magic(fp);	
	if (magic < 0) {
		res = LEV_IMG_ERR_HEADER;
		goto cleanup;
	}

	switch (magic) {
		case LEV_IMGFMT_PGM_ASCII: 
		case LEV_IMGFMT_PGM_RAW: {
			fseek(fp, 2, SEEK_SET);
			size_t max_val = 0;
			if (_lev_img_info_pgm(fp, out_width, out_height, &max_val) < 0) {
				res = LEV_IMG_ERR_HEADER;
				goto cleanup;
			}

			if (max_val > 255) 
				*out_bytes_per_pixel = 2;
			else
				*out_bytes_per_pixel = 1;
		} break;

		default: {
			res = LEV_IMG_ERR_UNKNOWN;
		} break;
	}
			
cleanup:
	if(fp) fclose(fp);
	return res;
}

int lev_img_load(const char* path, void *out_pixels, size_t buffer_size)
{
	if (!path || !out_pixels || buffer_size == 0)
		return LEV_ERR_INVALID;

	int res = 0;

	FILE *fp = fopen(path, "rb");
	if (!fp) {
		res = LEV_ERR_FOPEN;
		goto cleanup;
	}

	int magic = _lev_img_read_magic(fp);
	if (magic < 0) {
		res = LEV_IMG_ERR_HEADER;
		goto cleanup;
	}
	
	switch (magic) {
		case LEV_IMGFMT_PGM_RAW: {
			size_t max_val;
			if (_lev_img_skip_to_pixel(fp, &max_val) < 0) {
				res = LEV_ERR_READ;
				goto cleanup;
			}

			if (max_val > 255) {
				res = LEV_IMG_ERR_UNKNOWN;
				goto cleanup;
			}
						
			if (fread(out_pixels, 1, buffer_size, fp) != buffer_size) {
				res = LEV_ERR_READ;	
				goto cleanup;
			}

			if (max_val < 255) { // Scale up the value
				uint8_t *pixels = (uint8_t *)out_pixels;
				for (size_t i = 0; i < buffer_size; i++) {
					pixels[i] = (uint8_t)((pixels[i] * 255) / max_val);
				}
			}
		} break;

		case LEV_IMGFMT_PGM_ASCII: {
			size_t max_val;
			if (_lev_img_skip_to_pixel(fp, &max_val) < 0) {
				res = LEV_ERR_READ;
				goto cleanup;
			}

			if (max_val > 255) {
				res = LEV_IMG_ERR_UNKNOWN;
				goto cleanup;
			}

			uint8_t *pixels = (uint8_t *)out_pixels;

			for (size_t i = 0; i < buffer_size; i++) {
				unsigned int temp_val;

				if (fscanf(fp, "%u", &temp_val) != 1) {
					res = LEV_ERR_READ;	
					goto cleanup;
				}

				if (max_val == 255) 
					pixels[i] = (uint8_t)temp_val;
				else
					pixels[i] = (uint8_t)((temp_val * 255) / max_val);
			}
		} break;

		default: {
			res = LEV_IMG_ERR_UNKNOWN;
		} break;
	}

cleanup:
	if(fp) fclose(fp);
	return res;
}

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

void lev_str_tolower(char *str) 
{
	for (size_t i = 0; str[i] != '\0'; i++) {
		unsigned char c = (unsigned char)str[i];
		if (isalpha(c))
			str[i] = (char)tolower(c);
	}
}

void lev_str_alpha_only(char *str)
{
	int head = 0;
	int tail = 0;
	while (str[head] != '\0') {
		unsigned char c = (unsigned char)str[head];
		if (isalpha(c)) {
			str[tail] = str[head];
			tail++;
		}
		head++;
	}
	str[tail] = '\0';
}

size_t lev_file_size(const char *path)
{
	FILE *fp = fopen(path, "rb");
	if (!fp)
		return 0;

	fseek(fp, 0, SEEK_END);
	long s = ftell(fp);
	fclose(fp);
	
	if (s < 0)
		return 0;
	
	return (size_t) s;
}

int lev_file_read(const char *path, void *out_buffer, size_t buffer_size)
{
	if (!path || !out_buffer || buffer_size == 0)
		return LEV_ERR_INVALID;

	FILE *fp = fopen(path, "rb");
	if (!fp)
		return LEV_ERR_FOPEN;

	size_t read_bytes = fread(out_buffer, 1, buffer_size, fp);
	fclose(fp);

	if (read_bytes != buffer_size)
		return LEV_ERR_READ;
	
	return 0;
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

int lev_read_stdin(char *out_buffer, size_t buffer_size, int terminator)
{
	if (!out_buffer || buffer_size == 0)
		return LEV_ERR_INVALID;

	size_t i = 0;
	int c = EOF;

	while (i + 1 < buffer_size) {
		c = getchar();

		if (c == EOF) {
			if (i == 0) {
				out_buffer[0] = '\0';
				return LEV_ERR_READ;
			}
			break;
		}

		if (c == terminator)
			break;

		out_buffer[i++] = (char)c;
	}
	out_buffer[i] = '\0';

	if (i + 1 == buffer_size && c != terminator && c != EOF)
		return LEV_ERR_OVERFLOW;

	return (int)i;
}

const char *lev_error_msg(int error_code) 
{
	switch (error_code) {
		case LEV_ERR_INVALID: 	return "Invalid parameter passed.";
		case LEV_ERR_FOPEN:	return "Failed to open the file.";	
		case LEV_ERR_READ:	return "Error in reading the file.";
		case LEV_ERR_OVERFLOW:	return "Buffer overflow when storing to memory.";
		case LEV_IMG_ERR_HEADER:return "Error in reading the image header.";
		case LEV_IMG_ERR_UNKNOWN:return "Unknown or unsupported image format.";
		default:		return "Unknown error.";
	}
}
