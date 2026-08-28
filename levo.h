/*
===============================================================================
  Levo - Frictionless C utility library
===============================================================================
  No external dependencies, no complex compiler flags.
  Just copy levo.c and levo.h into your project and compile.

===============================================================================
  QUICK CHEAT SHEET
===============================================================================
  [String Utilities]
  lev_str_match()           - Pattern matching with '*' and '?'
  lev_str_tolower()	    - Change string to all lower case
  lev_str_alpha_only()      - Strip any character other than alphabets

  [Command-line Utilities]
  lev_cli_parse()           - Parse command-line options and arguments

  [OS Utilities]
  lev_os_getcwd()           - Get current working directory

  [Terminal Utilities]
  lev_term_enable_ansiesc() - Enable ANSI escape sequences (Windows)

  [General Utilities]
  lev_rand()                - Generate a pseudo-random integer
  lev_read_stdin()          - Read stdin until a specified terminator or EOF
  lev_printf_color()        - Print colored text to standard output

===============================================================================
*/

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

void lev_str_tolower(char *text); 
//TODO document me

void lev_str_alpha_only(char *text);
//TODO document me

// ============================================================================
// Command-line Utilities
// ============================================================================

// Command-line option definition and parse result container.
struct lev_cli_option {
	const char *name;       // option flag string (e.g. "-f", "--file")
	const char *value;      // pointer to the parsed value argument, or NULL
	bool        has_value;  // set to true if the option requires a trailing value
	bool        is_set;     // true if the option was matched during parsing
};

const char *lev_cli_parse(int argc, char *argv[], 
			  struct lev_cli_option opts[], // array of option definitions
			  size_t num_opts,              // number of elements in 'opts'
			  const char *rests[],          // array to receive positional arguments
			  int *rests_count);            // count of positional arguments
// Parses command-line arguments into options and positional arguments.
// Returns NULL on success, or a pointer to the unrecognized/invalid argument string.

// ============================================================================
// OS Utilities
// ============================================================================

char *lev_os_getcwd(char* out_buf, size_t size);
// Gets current working directory. Works on both UNIX and Windows.
// Returns pointer to 'out_buf' on success, otherwise NULL.

// ============================================================================
// Terminal Utilities
// ============================================================================

int lev_term_enable_ansiesc(void);
// Enables ANSI escape sequence in the console. 
// Required for Windows compatibility. On Linux/macOS, it's a safe no-op.
// Returns 0 on success, -1/-2 on win32 API failures.

// ============================================================================
// General Utilities
// ============================================================================

int lev_rand(uint32_t *seed, int min, int max);
// Generates a pseudo-random integer in the range [min, max] (inclusive).
// Uses XorShift32 internally. Updates the 32-bit 'seed' state on each call.

int lev_read_stdin(char *out_buf, size_t buf_size, int terminator);
// Reads stdin into 'out_buf' until 'terminator' char or EOF is encountered.
// Returns the number of bytes read (excluding null terminator).
// Returns negative values on error (-1: invalid param, -2: overflow, -3: EOF/error).

void lev_printf_color(int color, const char *fmt, ...);
// Prints colored text to standard output using ANSI 256-color codes.
// 'color' should be in the range 0-255.

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LEVO_H
