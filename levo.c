#include "levo.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

// ===========================================================================
// Image Processing
// ===========================================================================
enum lev_img_formats {
	LEV_IMGFMT_PBM_ASCII,
	LEV_IMGFMT_PBM_RAW,
	LEV_IMGFMT_PGM_ASCII,
	LEV_IMGFMT_PGM_RAW,
	LEV_IMGFMT_PPM_ASCII,
	LEV_IMGFMT_PPM_RAW,
};

static int _lev_img_read_magic(FILE *fp)
{
	// note: this chunk should be changed when parsing other img formats
	//     which uses magic number of more than 8 bytes
	uint8_t magic[8];
	size_t bytes_read = fread(magic, 1, 8, fp); 

	if (bytes_read != 8) {
		return -1;
	}

	if (magic[0] == 'P' && magic[1] == '1')
		return LEV_IMGFMT_PBM_ASCII;
	else if (magic[0] == 'P' && magic[1] == '2') 
		return LEV_IMGFMT_PGM_ASCII;
	else if (magic[0] == 'P' && magic[1] == '3')
		return LEV_IMGFMT_PPM_ASCII;
	else if (magic[0] == 'P' && magic[1] == '4')
		return LEV_IMGFMT_PBM_RAW;
	else if (magic[0] == 'P' && magic[1] == '5')
		return LEV_IMGFMT_PGM_RAW;
	else if (magic[0] == 'P' && magic[1] == '6')
		return LEV_IMGFMT_PPM_RAW;
	else
		return LEV_IMG_ERR_UNKNOWN;
}

// skip comment and whitespaces in pnm header
static int _lev_img_skip_pnm(FILE *fp)
{
	int c;
	while(1) {
		c = fgetc(fp);

		if (c == EOF) {
			return -1;
		} else if (c == '#') {
			// scan other than '\n' and dont store 
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

// max_val can be null
static int _lev_img_info_pnm(FILE *fp, size_t *width, size_t *height, size_t *max_val)
{
	if (_lev_img_skip_pnm(fp) != 0 || fscanf(fp, "%zu", width) != 1)
		return -1;	
	if (_lev_img_skip_pnm(fp) != 0 || fscanf(fp, "%zu", height) != 1)
		return -1;	
	if (max_val && (_lev_img_skip_pnm(fp) != 0 || fscanf(fp, "%zu", max_val) != 1)) 
		return -1;	

	(void)fgetc(fp); // dump the last spaces after max_val

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

	int fmt = _lev_img_read_magic(fp);	
	if (fmt < 0) {
		res = LEV_IMG_ERR_HEADER;
		goto cleanup;
	}

	switch (fmt) {
		case LEV_IMGFMT_PBM_ASCII:
		case LEV_IMGFMT_PBM_RAW: {
			fseek(fp, 2, SEEK_SET);
			if (_lev_img_info_pnm(fp, out_width, out_height, NULL) < 0) {
				res = LEV_IMG_ERR_HEADER;
				goto cleanup;
			}	
			*out_bytes_per_pixel = 1;
		}break;

		case LEV_IMGFMT_PGM_ASCII: 
		case LEV_IMGFMT_PGM_RAW: 
		case LEV_IMGFMT_PPM_ASCII:
		case LEV_IMGFMT_PPM_RAW: {
			fseek(fp, 2, SEEK_SET);
			size_t max_val = 0;
			if (_lev_img_info_pnm(fp, out_width, out_height, &max_val) < 0) {
				res = LEV_IMG_ERR_HEADER;
				goto cleanup;
			}

			if (max_val > 255) 
				*out_bytes_per_pixel =
					(fmt == LEV_IMGFMT_PPM_ASCII || fmt == LEV_IMGFMT_PPM_RAW) ? 6 : 2;
			else
				*out_bytes_per_pixel = 
					(fmt == LEV_IMGFMT_PPM_ASCII || fmt == LEV_IMGFMT_PPM_RAW) ? 3 : 1;
		} break;

		default: {
			res = LEV_IMG_ERR_UNKNOWN;
		} break;
	}
			
cleanup:
	if(fp) fclose(fp);
	return res;
}

// in_pixel & out_pixel have to be rgba order 
static void _lev_img_convert_channel(size_t o_chan, size_t d_chan, uint8_t *in_pixel, uint8_t *out_pixel) 
{
	uint8_t rgba[4] = {0, 0, 0, 255};

	if (o_chan == 1) {
		rgba[0] = rgba[1] = rgba[2] = in_pixel[0];
	} else if (o_chan == 2) {
		rgba[0] = rgba[1] = rgba[2] = in_pixel[0];
		rgba[3] = in_pixel[1];
	} else if (o_chan == 3) {
		rgba[0] = in_pixel[0];
		rgba[1] = in_pixel[1];
		rgba[2] = in_pixel[2];
	} else if (o_chan == 4) {
		rgba[0] = in_pixel[0];
		rgba[1] = in_pixel[1];
		rgba[2] = in_pixel[2];
		rgba[3] = in_pixel[3];
	}

	if (d_chan == 1) {
		//[ref] https://en.wikipedia.org/wiki/yiq
		out_pixel[0] = (0.299 * rgba[0] + 0.587 * rgba[1] + 0.114 * rgba[2]);
	} else if (d_chan == 2) {
		out_pixel[0] = (0.299 * rgba[0] + 0.587 * rgba[1] + 0.114 * rgba[2]);
		out_pixel[1] = rgba[3];
	} else if (d_chan == 3) {
		out_pixel[0] = rgba[0];
		out_pixel[1] = rgba[1];
		out_pixel[2] = rgba[2];
	} else if (d_chan == 4) {
		out_pixel[0] = rgba[0];
		out_pixel[1] = rgba[1];
		out_pixel[2] = rgba[2];
		out_pixel[3] = rgba[3];
	}
}

static int _lev_img_fill_chunk(FILE *fp, uint8_t *chunk, size_t *chunk_len, size_t *chunk_idx)
{
	if (*chunk_idx >= *chunk_len) {
		*chunk_len = fread(chunk, 1, BUFSIZ, fp);
		*chunk_idx = 0;
		if (*chunk_len == 0)
			return LEV_ERR_READ;
	}

	return 0;
}

static int _lev_img_load_pnm(int fmt, FILE *fp, void *out_pixels, size_t buffer_size, int desired_channel)
{
	fseek(fp, 2, SEEK_SET);
	size_t width, height, max_val = 255;

	if (fmt == LEV_IMGFMT_PBM_ASCII || fmt == LEV_IMGFMT_PBM_RAW) {
		if (_lev_img_info_pnm(fp, &width, &height, NULL) < 0)
			return LEV_IMG_ERR_HEADER;
	} else {
		if (_lev_img_info_pnm(fp, &width, &height, &max_val) < 0)
			return LEV_IMG_ERR_HEADER;
	}

	size_t o_chan = (fmt == LEV_IMGFMT_PPM_ASCII || fmt == LEV_IMGFMT_PPM_RAW) ? 3 : 1;
	size_t d_chan = (desired_channel == 0) ? o_chan : (size_t)desired_channel; 

	if (width * height * d_chan > buffer_size)
		return LEV_ERR_OVERFLOW;

	uint8_t *pixels = (uint8_t *)out_pixels;
	size_t out_idx = 0;
	uint8_t pbmr_buffer = 0;

	uint8_t chunk[BUFSIZ];
	size_t chunk_len = 0;
	size_t chunk_idx = BUFSIZ;

	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			uint8_t in_pixel[4] = {0};

			switch (fmt) {
				case LEV_IMGFMT_PGM_RAW:
				case LEV_IMGFMT_PPM_RAW: {
					for (size_t c = 0; c < o_chan; c++) {
						if (_lev_img_fill_chunk(fp, chunk, &chunk_len, &chunk_idx) < 0)
							return LEV_ERR_READ;
						in_pixel[c] = chunk[chunk_idx++];
					}
				} break;

				case LEV_IMGFMT_PBM_RAW: {
					// 8 pixels are packed in 1byte in pbm binary format
					if (x % 8 == 0) {
						if (_lev_img_fill_chunk(fp, chunk, &chunk_len, &chunk_idx) < 0)
							return LEV_ERR_READ;
						pbmr_buffer = chunk[chunk_idx++];
					}
					in_pixel[0] = ((pbmr_buffer >> (7 - (x % 8))) & 0x01) ? 0 : 255;
				} break;

				case LEV_IMGFMT_PGM_ASCII:
				case LEV_IMGFMT_PPM_ASCII:
				case LEV_IMGFMT_PBM_ASCII: {
					for (size_t c = 0; c < o_chan; c++) {
						int ch;
						while((ch = fgetc(fp)) != EOF && isspace(ch));
						if (ch == EOF)
							return LEV_ERR_READ;

						unsigned int tmp = 0;
						while (ch >= '0' && ch <= '9') {
							tmp = tmp * 10 + (ch - '0');
							ch = fgetc(fp);
						}
						if (ch != EOF)
							ungetc(ch, fp);

						if (fmt != LEV_IMGFMT_PBM_ASCII)
							in_pixel[c] = (uint8_t)tmp;
						else
							in_pixel[c] = (tmp == 1) ? 0 : 255;
					}
				} break;
			}

			if (max_val < 255) {
				for (size_t j = 0; j < o_chan; j++)
					in_pixel[j] = (uint8_t)((in_pixel[j] * 255) / max_val);
			}

			_lev_img_convert_channel(o_chan, d_chan, in_pixel, &pixels[out_idx]);
			out_idx += d_chan;
		}
	}

	return 0;
}

int lev_img_load(const char* path, void *out_pixels, size_t buffer_size, int desired_channel)
{
	if (!path || !out_pixels || buffer_size == 0 || desired_channel < 0 || desired_channel > 4)
		return LEV_ERR_INVALID;

	int res = 0;

	FILE *fp = fopen(path, "rb");
	if (!fp) {
		res = LEV_ERR_FOPEN;
		goto cleanup;
	}

	int fmt = _lev_img_read_magic(fp);
	if (fmt < 0) {
		res = LEV_IMG_ERR_HEADER;
		goto cleanup;
	}
	
	switch (fmt) {
		case LEV_IMGFMT_PBM_RAW:
		case LEV_IMGFMT_PPM_RAW:
		case LEV_IMGFMT_PGM_RAW: 
		case LEV_IMGFMT_PBM_ASCII: 
		case LEV_IMGFMT_PPM_ASCII:
		case LEV_IMGFMT_PGM_ASCII: {
			if ((res = _lev_img_load_pnm(fmt, fp, out_pixels, buffer_size, desired_channel)) < 0) 
				goto cleanup;	
		} break;

		default: {
			res = LEV_IMG_ERR_UNKNOWN;
		} break;
	}

cleanup:
	if(fp) fclose(fp);
	return res;
}

// ===========================================================================
// 2D Drawing 
// ===========================================================================

static void _lev_draw_get_rgba(uint32_t color, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a)
{
	*r = (color >> (8 * 3)) & 0xff;
	*g = (color >> (8 * 2)) & 0xff;
	*b = (color >> (8 * 1)) & 0xff;
	*a = (color >> (8 * 0)) & 0xff;
}

int lev_draw_fill(void *pixels, size_t width, size_t height, uint32_t color)
{
	if (!pixels)
		return LEV_ERR_INVALID;

	uint8_t *p = (uint8_t *)pixels;
	uint8_t r,g,b,a;
	_lev_draw_get_rgba(color, &r, &g, &b, &a);

	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			size_t idx = (y * width + x) * 4;
			p[idx + 0] = r;
			p[idx + 1] = g;
			p[idx + 2] = b;
			p[idx + 3] = a;
		}
	}

	return 0;
}

int lev_draw_fillrect(void *pixels, size_t pixel_w, size_t pixel_h, int x, int y, size_t rect_w, size_t rect_h, uint32_t color)
{
	if(!pixels)
		return LEV_ERR_INVALID;

	uint8_t *p = (uint8_t *)pixels;
	uint8_t r,g,b,a;
	_lev_draw_get_rgba(color, &r, &g, &b, &a);
	
	int x0 = (x > 0) ? x : 0;
	int y0 = (y > 0) ? y : 0; 
	int x1 = ((x + (int)rect_w) < (int)pixel_w) ? x + (int)rect_w : (int)pixel_w;
	int y1 = ((y + (int)rect_h) < (int)pixel_h) ? y + (int)rect_h : (int)pixel_h;

	for (int ry = y0; ry < y1; ry++) {
		for (int rx = x0; rx < x1; rx++) {
			size_t idx = ((size_t)ry * pixel_w + (size_t)rx) * 4;
			p[idx + 0] = r;
			p[idx + 1] = g;
			p[idx + 2] = b;
			p[idx + 3] = a;
		}
	}
	
	return 0;
}

// ===========================================================================
// String Processing
// ===========================================================================
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

// ===========================================================================
// File Utilities
// ===========================================================================
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

// ===========================================================================
// Command-line Utilities
// ===========================================================================
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

// ===========================================================================
// General Utilities
// ===========================================================================
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

// ===========================================================================
// Others
// ===========================================================================
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
