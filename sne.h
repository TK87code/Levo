/**
 * @file sne.h
 * @brief simple n easy library.
 */
#ifndef SNE_H 
#define SNE_H 

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**************************************************************************************************************
 * @defgroup String String Utilities(sne_str)
 * @{
 */
	
// todo document
bool sne_str_match(const char *pattern, const char *str);

/******************************************************************************************************** @} */


/***************************************************************************************************************
 * @defgroup Commandline Command line Utilities(sne_cli)
 * @{
 */

/**
 * @brief command-line option definition and parse result container.
 */
struct sne_cl_option {
	const char *name;       /**< option flag string (e.g. "-f", "--file"). */
	const char *value;      /**< pointer to the parsed value argument, or null. */
	bool        has_value;  /**< set to true if the option requires a trailing value. */
	bool        is_set;     /**< true if the option was matched during parsing. */
};

/**
 * @brief  parse command-line arguments into options and positional arguments.
 *
 * @param  argc        argument count from main().
 * @param  argv        argument vector from main().
 * @param  opts        array of option definitions (will be updated with results).
 * @param  num_opts    number of elements in the opts array.
 * @param  rests       array to receive pointers to positional (non-option) arguments.
 * @param  rests_count pointer to an int that will receive the count of positional arguments.
 *
 * @return null on success, or a pointer to the unrecognized/invalid argument string.
 */
const char *sne_cli_parse(int argc, char *argv[], struct sne_cl_option opts[], size_t num_opts, const char *rests[], int *rests_count);

/*********************************************************************************************************** @} */


/***************************************************************************************************************
 * @defgroup OS OS Utilities(sne_os)
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
char *sne_os_getcwd(char* out_buf, size_t size);

/*********************************************************************************************************** @} */


/***************************************************************************************************************
 * @defgroup General General Utilities(sne)
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
int sne_rand(uint32_t *seed, int min, int max);

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
int sne_read_stdin(char *out_buf, size_t buf_size, int terminator);

/*********************************************************************************************************** @} */

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SNE_H 

