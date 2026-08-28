/**
 * @file lev.h
 * @brief simple n easy library.
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

/**************************************************************************************************************
 * @defgroup String String Utilities(lev_str)
 * @{
 */
	
//TODO document this
bool lev_str_match(const char *pattern, const char *str);

/******************************************************************************************************** @} */


/***************************************************************************************************************
 * @defgroup Command-line Command line Utilities(lev_cli)
 * @{
 */

/**
 * @brief Command-line option definition and parse result container.
 */
struct lev_cli_option {
	const char *name;       /**< Option flag string (e.g. "-f", "--file"). */
	const char *value;      /**< Pointer to the parsed value argument, or null. */
	bool        has_value;  /**< Set to true if the option requires a trailing value. */
	bool        is_set;     /**< True if the option was matched during parsing. */
};

/**
 * @brief  Parses command-line arguments into options and positional arguments.
 *
 * @param  argc        Argument count from main().
 * @param  argv        Argument vector from main().
 * @param  opts        Array of option definitions (will be updated with results).
 * @param  num_opts    Number of elements in the opts array.
 * @param  rests       Array to receive pointers to positional (non-option) arguments.
 * @param  rests_count Pointer to an int that will receive the count of positional arguments.
 *
 * @return null on success, or a pointer to the unrecognized/invalid argument string.
 */
const char *lev_cli_parse(int argc, char *argv[], struct lev_cli_option opts[], size_t num_opts, const char *rests[], int *rests_count);

/*********************************************************************************************************** @} */


/***************************************************************************************************************
 * @defgroup OS OS Utilities(lev_os)
 * @{ 
 */

/**
 * @brief  Get current working directly. Can be used for both UNIX and Windows
 *
 * @param  buffer Pointer to the buffer to store path. 
 * @param  size Size of the buffer.
 *
 * @return Pointer to the buffer that stores the path when succeeded, otherwise NULL. 
 */
char *lev_os_getcwd(char* out_buf, size_t size);

/*********************************************************************************************************** @} */

/***************************************************************************************************************
 * @defgroup Terminal Terminal Utilities(lev_term)
 * @{ 
 */

/**
 * @brief Enable ANSI escape sequence in the console.
 * @note  This is required for Windows compatibility. On Linux/macOS, this function is a no-op and can be omitted.
 *
 * @retval 0 Success
 * @retval -1 Failed in GetStdHandle() of win32 API.
 * @retval -2 Failed in GetConsoleMode() of win32 API. 
 */
int lev_term_enable_ansiesc(void);

/*********************************************************************************************************** @} */

/***************************************************************************************************************
 * @defgroup General General Utilities(lev)
 * @{ 
 */

/**
 * @brief  Generate a pseudo-random integer in the range [min, max] using XorShift32.
 *
 * @param  seed Pointer to a 32-bit seed state (updated on each call).
 * @param  min  Minimum number (inclusive).
 * @param  max  Maximum number (inclusive).
 *
 * @return A pseudo-random integer between min and max (inclusive).
 */
int lev_rand(uint32_t *seed, int min, int max);

/**
 * @brief  Read stdin until a specified terminator or EOF.
 *
 * @param  out_buf    A buffer to store the null-terminated string.
 * @param  buf_size   Total capacity of out_buf in bytes.
 * @param  terminator Delimiter character to stop reading (e.g. '\n').
 * 
 * @return The number of bytes read (excluding the null terminator).
 * @retval -1 Invalid parameter (e.g. out_buf is NULL or buf_size is 0).
 * @retval -2 Buffer overflow (buffer filled before terminator or EOF was encountered).
 * @retval -3 Stream ended before reading any character, or an I/O error occurred.
 */
int lev_read_stdin(char *out_buf, size_t buf_size, int terminator);

//TODO Document it
void lev_printf_color(int color, const char *fmt, ...);

/*********************************************************************************************************** @} */

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LEVO_H 

