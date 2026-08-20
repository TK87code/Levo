#ifndef MTB_H
#define MTB_H

#include <stdint.h>

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
