// Peeki.c - A simple image viewer(peeker) using Levo image module
// - This is just a quick way to peek inside a image file.
// - Drop a file to open, mouse wheel to zoom, drag to move, escape to reset zoom. That's about it.
// Last Update: 2026-09-04 

#include "levo.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>

#define WIN_WIDTH 960 
#define WIN_HEIGHT 720 

struct state {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	size_t w, h, bpp;
	size_t img_size;
	size_t win_w, win_h;
	uint32_t *pixels;
	float scale;
	const float zoom_factor;
};

static int load_image(struct state *s, const char *path)
{
	if(lev_img_info(path, &s->w, &s->h, &s->bpp) < 0)  
		return -1;

	s->img_size = s->w * s->h * sizeof(uint32_t);
	s->pixels = malloc(s->img_size);
	if (!s->pixels) 
		return -1;

	if(lev_img_load(path, s->pixels, s->img_size, 4) < 0)
		return -1;

	return 0;
}

static void main_loop(struct state *s)
{
	bool is_running = true;
	bool is_dragging = false;
	SDL_Event event;

	int imgx = (s->win_w - s->w) / 2;
	int imgy = (s->win_h - s->h) / 2;

	while (is_running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				is_running = false;

			} else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) { 
					s->scale = 1.0f;
					imgx = (s->win_w - s->w) / 2;
					imgy = (s->win_h - s->h) / 2;
				}

			} else if (event.type == SDL_MOUSEWHEEL) {
				int mouse_x, mouse_y;
				SDL_GetMouseState(&mouse_x, &mouse_y);

				float old_scale = s->scale;

				if (event.wheel.y > 0)
					s->scale *= s->zoom_factor;
				else if (event.wheel.y < 0) 
					s->scale *= (1 / s->zoom_factor);	

				float scale_ratio = s->scale / old_scale;

				imgx = mouse_x - (mouse_x - imgx) * scale_ratio;
				imgy = mouse_y - (mouse_y - imgy) * scale_ratio;
				
			} else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
				is_dragging = true;
			} else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
				is_dragging = false;	
			} else if (event.type == SDL_MOUSEMOTION && is_dragging) {
				imgx += event.motion.xrel;
				imgy += event.motion.yrel;
			} else if (event.type == SDL_DROPFILE) {
				char *path = event.drop.file;
				if(path) {
					if (s->pixels) {
						free(s->pixels);
						s->pixels = NULL;
					}

					if (s->texture) {
						SDL_DestroyTexture(s->texture);
						s->texture = NULL;
					}

					if (load_image(s, path) < 0) {
						SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, 
								"Error", "Failed to load the image file", NULL);
					}
					
					s->scale = 1.0f;
					imgx = (s->win_w - s->w) / 2;
					imgy = (s->win_h - s->h) / 2;
					s->texture = SDL_CreateTexture(s->renderer, 
						SDL_PIXELFORMAT_RGBA32, 
						SDL_TEXTUREACCESS_TARGET, 
						s->w, s->h);	

					if (s->texture != NULL)
						SDL_UpdateTexture(s->texture, NULL, s->pixels, 
								s->w * sizeof(uint32_t));
					SDL_free(path);
					SDL_RaiseWindow(s->window);
				}
			}
		}

		SDL_SetRenderDrawColor(s->renderer, 30, 30, 30, 255);
		SDL_RenderClear(s->renderer);

		SDL_Rect dist = {
			.x = imgx,
			.y = imgy,
			.w = s->w * s->scale,
			.h = s->h * s->scale,
		};
		SDL_RenderCopy(s->renderer,s->texture, NULL, &dist);

		SDL_RenderPresent(s->renderer);
		SDL_Delay(16); 
	}
}

static int setup_window(struct state *s);
static void sdl_cleanup(struct state *s);

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	struct state s = {
		.scale = 1.0f,
		.zoom_factor = 1.1f,
	};

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		//TODO error messaging
		goto cleanup;

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

	if (setup_window(&s) < 0)
		//TODO error messaging
		goto cleanup;
	
	main_loop(&s);

cleanup:
	sdl_cleanup(&s);
	if (s.pixels)
		free(s.pixels);
	return 0;
}

static int setup_window(struct state *s)
{
	s->win_w = WIN_WIDTH;		
	s->win_h = WIN_HEIGHT;		


	s->window = SDL_CreateWindow(
			"Peeki - Drop a image inside window -",
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
