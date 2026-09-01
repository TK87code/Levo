//===============================================================================
//  Levo - Frictionless C utility library 
//===============================================================================
//  No external dependencies, no complex compiler flags.
//  This library relies exclusively on C99 standard library functions.
//  Just copy levo.c and levo.h into your project and compile.
//===============================================================================
//  QUICK CHEAT SHEET
//===============================================================================
//  			[String Utilities]
//
//  lev_str_match()           	- Pattern matching with '*' and '?'
//  lev_str_tolower()	    	- Change string to all lower case
//  lev_str_alpha_only()      	- Strip any character other than alphabets
//
//  			[File Utilities]
//
//  lev_file_size()	      	- Read file size in bytes
//  lev_file_read()		- Read specified bytes of data from a file
//
//  			[Command-line Utilities]
//
//  lev_cli_parse()           	- Parse command-line options and arguments
//
//  			[General Utilities]
//
//  lev_rand()                	- Generate a pseudo-random integer
//  lev_read_stdin()          	- Read stdin until a specified terminator or EOF
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

// ============================================================================
// String Utilities
// ============================================================================

bool lev_str_match(const char *pattern, const char *str);
// Matches a string against a pattern using '*' (any sequence) and '?' (any single character).
// Returns true if matched, false otherwise.

void lev_str_tolower(char *str); 
// REPLACE given string to all lowercase.

void lev_str_alpha_only(char *str);
// Strip all characters other than alphabets from the given string and REPLACE it. 

// ============================================================================
// String Utilities
// ============================================================================

size_t lev_file_size(const char *path);
// Returns the size of the file in bytes. Returns 0 on failure.

int lev_file_read(const char *path, void *out_buffer, size_t buffer_size);
// Reads specified bytes from the file, and store them to the 'out_buffer'
// Returns 0 on success. 
// Errors-> -1:invalid parameter -2: Failed to open file -3: Could not read full bytes 
	
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

int lev_rand(uint32_t *seed, int min, int max);
// Generates a pseudo-random integer in the range [min, max] (inclusive).
// Uses XorShift32 internally. Updates the 32-bit 'seed' state on each call.

int lev_read_stdin(char *out_buffer, size_t buffer_size, int terminator);
// Reads stdin into 'out_buffer' until 'terminator' char or EOF is encountered.
// Returns the number of bytes read (excluding null terminator).
// Returns negative values on error (-1: invalid param, -2: overflow, -3: EOF/error).

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LEVO_H
