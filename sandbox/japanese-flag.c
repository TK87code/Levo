// japanese-flag.c (test of lev_img_write_ppm)
// - Description: Draw Japense flag on memory and writes ppm image file at current directly.
// - Last edit: 2026.09.09
#include "levo.h"
#include <stdint.h>
#include <stdlib.h>

int main(void)
{
	size_t width = 800;
	size_t height = 600;
	int channel = 4;
	size_t buff_size = width * height * channel;
	uint32_t *buffer = malloc(buff_size); 
	if (!buffer)
		return 1;
		
	lev_draw_fill(buffer, width, height, 0xffffffff);
	lev_draw_circle(buffer, width, height, width / 2, height / 2, 100, 0xff0000ff);

	lev_img_write_ppm("flag.ppm", buffer, width, height, channel);

	free(buffer);
	return 0;
}
