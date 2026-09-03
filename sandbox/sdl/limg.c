#include "levo.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>

#define MIN_WIN_WIDTH 800
#define MIN_WIN_HEIGHT 600

struct state {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	size_t w, h, bpp;
	size_t img_size;
	size_t win_w, win_h;
	uint8_t *pixels;
	uint32_t *pixels32;
};

static void main_loop(struct state *s)
{
	bool is_running = true;
	SDL_Event event;
	while (is_running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) 
				is_running = false;

			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) 
					is_running = false;
			}
		}

		SDL_SetRenderDrawColor(s->renderer, 30, 30, 30, 255);
		SDL_RenderClear(s->renderer);
		SDL_Rect dist = {
			.x = (s->win_w - s->w) / 2,
			.y = (s->win_h - s->h) / 2,
			.w = s->w,
			.h = s->h,
		};
		SDL_RenderCopy(s->renderer,s->texture, NULL, &dist);

		SDL_RenderPresent(s->renderer);
		SDL_Delay(16); 
	}
}

static int setup_window(struct state *s)
{
	s->win_w = (s->w < MIN_WIN_WIDTH) ? MIN_WIN_WIDTH : s->w;		
	s->win_h = (s->h < MIN_WIN_HEIGHT) ? MIN_WIN_HEIGHT : s->h;		

	s->window = SDL_CreateWindow(
			"SDL Quick Start",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			s->win_w,
			s->win_h,
			SDL_WINDOW_SHOWN
			);

	if (!s->window) {
		printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	s->renderer = SDL_CreateRenderer(s->window, -1, SDL_RENDERER_ACCELERATED);
	if (!s->renderer) {
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(s->window);
		SDL_Quit();
		return -1;
	}

	return 0;
}

static void sdl_cleanup(struct state *s)
{
	SDL_DestroyTexture(s->texture);
	SDL_DestroyRenderer(s->renderer);
	SDL_DestroyWindow(s->window);
	SDL_Quit();
}

int main(int argc, char *argv[])
{
	if (argc != 2)
		return -1;

	struct state s = {0};
	int res = 0;

	if((res = lev_img_info(argv[1], &s.w, &s.h, &s.bpp)) < 0) {
		printf("%s",lev_error_msg(res));
		return -1;
	}
	
	s.img_size = s.w * s.h * s.bpp;
	s.pixels = malloc(s.img_size);
	s.pixels32 = malloc(s.w * s.h * sizeof(uint32_t)); 
	if (!s.pixels || !s.pixels32) {
		printf("Out of memory.");
		res = -1;
		goto cleanup;
	}

	if((res = lev_img_load(argv[1], s.pixels, s.img_size)) < 0) {
		printf("%s", lev_error_msg(res));
		res = -1;
		goto cleanup;
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL_Init Error: %s\n", SDL_GetError());
		res = -1;
		goto cleanup;
	}

	if (setup_window(&s) < 0) {
		printf("Error setting up SDL window");
		res = -1;
		goto cleanup;
	}

	for (size_t i = 0; i < (s.w * s.h); i++) {
		uint8_t v = s.pixels[i];
		s.pixels32[i] = 255 << (8 * 0);
		s.pixels32[i] |= v << (8 * 1);
		s.pixels32[i] |= v << (8 * 2);
		s.pixels32[i] |= v << (8 * 3);
	}

	s.texture = SDL_CreateTexture(s.renderer, 
						SDL_PIXELFORMAT_RGBA8888, 
						SDL_TEXTUREACCESS_TARGET, 
						s.w, s.h);	
	SDL_UpdateTexture(s.texture,
                      	  NULL,
                      	  s.pixels32, s.w * sizeof(uint32_t));

	if (s.texture == NULL) {
		printf("Error in Texture creation.");
		res = -1;
		goto cleanup;
	}

	main_loop(&s);

cleanup:
	sdl_cleanup(&s);
	if (s.pixels)
		free(s.pixels);
	if (s.pixels32)
		free(s.pixels32);

	return 0;
}
