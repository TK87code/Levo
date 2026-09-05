//===============================================================================
//  Levo - Frictionless C utility library 
//===============================================================================
//  No external dependencies, no complex compiler flags.
//  This library relies exclusively on C99 standard library functions.
//  Just copy levo.c and levo.h into your project and compile.
//===============================================================================
//  QUICK CHEAT SHEET
//===============================================================================
//			[ Image Processing ] (lev_img_xx)
//
//  lev_img_info()		- Get width, height, and bpp of an image
//  lev_img_load()		- Load 8bit/channel image into memory
//
//  			[ 2D Drawing ] (lev_draw)
//
//  lev_draw_fill()		- Fill pixels with color
//  lev_draw_rect()		- Fill pixels inside a rectangle with color 
//  lev_draw_line()		- Draw a line with color
//
//  			[ String Processing ] (lev_str_xx)
//
//  lev_str_match()           	- Pattern matching with '*' and '?'
//  lev_str_tolower()	    	- Change string to all lower case
//  lev_str_alpha_only()      	- Strip any character other than alphabets
//
//  			[ File Utilities ] (lev_file_xx)
//
//  lev_file_size()	      	- Read file size in bytes
//  lev_file_read()		- Read specified bytes of data from a file
//
//  			[ Command-line Utilities ] (lev_cli_xx)
//
//  lev_cli_parse()           	- Parse command-line options and arguments
//
//  			[ General Utilities ] (lev_xx)
//
//  LEV_SWAP()			- Swap the value of two variables
//  lev_rand()                	- Generate a pseudo-random integer
//  lev_read_stdin()          	- Read stdin until a specified terminator or EOF
//
//			[ Others ] (lev_xx)
//			
//  lev_error_msg()		- Return error message from Levo
//
//===============================================================================

#ifndef LEVO_H 
#define LEVO_H 

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

enum lev_errors {
	LEV_ERR_INVALID = -1,
	LEV_ERR_FOPEN = -2,
	LEV_ERR_READ = -3,
	LEV_ERR_OVERFLOW = -4,
	LEV_IMG_ERR_HEADER = -5,
	LEV_IMG_ERR_UNKNOWN = -6,
};

// ============================================================================
// Image Processing 
// ============================================================================

int lev_img_info(const char *path, size_t *out_width, size_t *out_height, size_t *out_bytes_per_pixel);
// Read pixel width, pixel height, bytes-per-pixel data from the header of an image file,
// 	and store it to given memory address.
// Use this to calculate buffer size needed before call lev_img_load() if the size is unknown.
// Return 0 on success, or negative error code on failure. 

int lev_img_load(const char* path, void *out_pixels, size_t buffer_size, int desired_channel);
// [8 bit-per-channel only] Read pixel data from an image file, and store them to given memory address.
// Return 0 on success, or negative error code on failure.
//
// [Channels]
// Specify 'desired_channel' to force conversion, or 0 to keep the original channels:
// 	0 : Same as original
// 	1 : Luminance(Y) 
// 	2 : Luminance(Y) + Alpha (255)
// 	3 : RGB
// 	4 : RGBA (Alpha is always 255)
// Greyscaling is done by "The Rec. 601 luma formula"
//
// [Memory Layout]
// The output is written as a tightly packed array of unsigned 8-bit integers(uint8_t).
// Pixels are stored row by row, from top to bottom, left to right.
// For example, if desired_channel = 4, the memory layout will be as follows:
// (R, G, B, A, R, G, B, A, ...)
//
// [Supported format]
// PBM, PGM, PPM (Both ASCII and RAW/Binary):
// 	Pixels with a maxval other than 255 will be normalized to 0-255.	

// ============================================================================
// 2D Drawing 
// ============================================================================

int lev_draw_fill(void *pixels, size_t width, size_t height, uint32_t color);
// Fills pixels with specified color. 4 byte color need to be in order of RGBA.
// For example, If you need to fill the buffer with solid red, it should be
// Specified as '0xFF0000FF'.
// Return 0 on success, or negative error code on failure.

int lev_draw_rect(void *pixels, size_t pixel_w, size_t pixel_h, // Data of the buffer storing pixels
		  int x, int y, size_t rect_w, size_t rect_h,   // Data of the rectangle to draw
		  uint32_t color);				// Color need to be RGBA order. (e.g., Solid red -> 0xff0000ff)
// Fills pixels inside rectangle with color.
// Return 0 on success, or negative error code on failure

int lev_draw_line(void *pixels, size_t pixel_w, size_t pixel_h,	// Data of the buffer storing pixels
	          int x0, int y0,				// The first coordinate
		  int x1, int y1,				// The second coordinate 
		  uint32_t color);				// Color need to be RGBA order. (e.g., Solid red -> 0xff0000ff)
// Draws a line between the specified  2 coordinates.
// Return 0 on success, or negative error code on failure

// ============================================================================
// String Processing 
// ============================================================================

bool lev_str_match(const char *pattern, const char *str);
// Matches a string against a pattern using '*' (any sequence) and '?' (any single character).
// Returns true if matched, false otherwise.

void lev_str_tolower(char *str); 
// REPLACE given string to all lowercase.

void lev_str_alpha_only(char *str);
// Strip all characters other than alphabets from the given string and REPLACE it. 

// ============================================================================
// File Utilities
// ============================================================================

size_t lev_file_size(const char *path);
// Returns the size of the file in bytes. Returns 0 on failure.

int lev_file_read(const char *path, void *out_buffer, size_t buffer_size);
// Reads specified bytes from the file, and store them to the 'out_buffer'
// Returns 0 on success, or negative error code on failure.
	
// ============================================================================
// Command-line Utilities
// ============================================================================

// Command-line option definition and parse result container.
struct lev_cli_option {
	const char *name;       // Option flag string (e.g. "-f", "--file")
	const char *value;      // Pointer to the parsed value argument, or NULL
	bool        has_value;  // Set to true if the option requires a trailing value
	bool        is_set;     // True if the option was matched during parsing
};

const char *lev_cli_parse(int argc, char *argv[],	// argc & argv from main 
			  struct lev_cli_option opts[], // array of option definitions
			  size_t num_opts,              // number of elements in 'opts'
			  const char *rests[],          // array to receive positional arguments
			  int *rests_count);            // count of positional arguments
// Parses command-line arguments into options and positional arguments.
// Returns NULL on success, or a pointer to the unrecognized/invalid argument string.

// ============================================================================
// General Utilities
// ============================================================================

#define LEV_SWAP(type, a, b) do { type _tmp = (a); (a) = (b); (b) = _tmp; } while (0)
// Swaps the value of two variables of the specified 'type'.
// Usage: LEV_SWAP(int, x0, x1);

int lev_rand(uint32_t *seed, int min, int max);
// Generates a pseudo-random integer in the range [min, max] (inclusive).
// Uses XorShift32 internally. Updates the 32-bit 'seed' state on each call.

int lev_read_stdin(char *out_buffer, size_t buffer_size, int terminator);
// Reads stdin into 'out_buffer' until 'terminator' char or EOF is encountered.
// Returns the number of bytes read (excluding null terminator), 
//	or nagative error code on failure. 

// ============================================================================
// General Utilities
// ============================================================================

const char *lev_error_msg(int error_code);
// Generate error message of error_code.

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LEVO_H
