#ifndef MTB_H
#define MTB_H

#include <stddef.h>
#include <stdint.h>

/* @defgroup Console
 * @{
 */

/**
 * @brief  Read stdin untill a specified terminator.
 *
 * @param  out_buf A buffer to store read string.
 * @param  buf_size Size of the out_buf.
 * @param  terminator Terminator.
 * 
 * @return The number of bytes read. Errors: -1 Invalid parameter. -2 stopped reading by buffer overflow. 
 * @retval -1 Invalid parameter.
 * @retval -2 Buffer overflow. Input remained in stdin.
 * @retval -3 Stream stopped without reading any character, or I/O error occured.
 */
int mtb_read_stdin(char *buffer, size_t buf_size, int terminator);

/** @} */

/* @defgroup Utils
 * @{ 
 */

/**
 * @brief  Generate random number in the range. Using XorShift algorithm.
 *
 * @param  seed A pointer to 32bit seed number.
 * @param  min Minimum number.
 * @param  max Maximum number.
 *
 * @return A random number from min to max.
 */
int mtb_rand(uint32_t *seed, int min, int max);

/** @} */
#endif // MTB_H
