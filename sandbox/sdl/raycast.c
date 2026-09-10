#include "levo.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>

//#define RETRO

#define PI 3.14159265

#define MAP_ROWS 13 
#define MAP_COLS 20

#define MINI_MAP_SCALE 0.1

#ifdef RETRO
	#define RES_WIDTH 320 
	#define RES_HEIGHT 200 
	#define TILE_SIZE 32 
#else 
	#define RES_WIDTH 960 
	#define RES_HEIGHT 540 
	#define TILE_SIZE 64 
#endif // RETRO

#define WIN_WIDTH 1280 
#define WIN_HEIGHT 720 

#define MAP_WIDTH MAP_COLS * TILE_SIZE
#define MAP_HEIGHT MAP_ROWS * TILE_SIZE

#define FOV_ANGLE (60 * (PI / 180))

#define NUM_RAYS RES_WIDTH

#define FPS 60

const int map[MAP_ROWS][MAP_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

struct player {
	float x, y;
	float r;
	int turn_direction; // -1 for left, +1 for right
	int walk_direction; // -1 for back, +1 for front
	float rotation_angle;
	float walk_speed;
	float turn_speed;
};

struct rays {
	float angle[NUM_RAYS];
	float wallhit_x[NUM_RAYS];
	float wallhit_y[NUM_RAYS];
	float distance_sq[NUM_RAYS];
	bool is_hit_wall[NUM_RAYS];
	bool is_hit_vertwall[NUM_RAYS];
};

struct state {
	struct player p;
	struct rays rays;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	uint32_t *pixels;
	float ticks_lastframe;
};

static int sdl_setup(struct state *s);
static void sdl_cleanup(struct state *s);
static void main_loop(struct state *s);

void move_player(struct state *s, float dt) 
{
	s->p.rotation_angle += s->p.turn_direction * s->p.turn_speed * dt;
	float move_step = s->p.walk_direction * s->p.walk_speed * dt;

	float newx = s->p.x + cos(s->p.rotation_angle) * move_step;
	float newy = s->p.y + sin(s->p.rotation_angle) * move_step;

	if (map[(int)(newy / TILE_SIZE)][(int)(newx / TILE_SIZE)] != 1) {
		s->p.x = newx;
		s->p.y = newy;
	}
}

void detect_horizontal_intercept(struct state *s, int ray_id, float ray_sin, float ray_cos) 
{
	float angle = s->rays.angle[ray_id];
	bool is_facing_down = (angle > 0 && angle < PI);
	bool is_facing_up = !is_facing_down;
	bool is_facing_right = (angle < PI * 0.5 || angle > 1.5 * PI);
	bool is_facing_left = !is_facing_right;

	s->rays.is_hit_wall[ray_id] = false;
		
	float yintercept = floor(s->p.y / TILE_SIZE) * TILE_SIZE;
	yintercept += (is_facing_down) ? TILE_SIZE : 0;

	float xintercept = s->p.x + (yintercept - s->p.y) * (ray_cos / ray_sin);

	float ystep = TILE_SIZE;
	ystep *= (is_facing_down) ? 1 : -1;

	float xstep = TILE_SIZE * (ray_cos / ray_sin);
	xstep *= (is_facing_right && xstep < 0) ? -1 : 1; 
	xstep *= (is_facing_left && xstep > 0) ? -1 : 1;

	float next_checkx = xintercept;
	float next_checky = yintercept;

	while (1) {
		int map_x = (int)(next_checkx / TILE_SIZE);
		int map_y = (int)((next_checky + (is_facing_up ? -1 : 0)) / TILE_SIZE);

		if (map_x < 0 || map_x >= MAP_COLS || map_y < 0 || map_y >= MAP_ROWS || map[map_y][map_x] == 1) {
			s->rays.is_hit_wall[ray_id] = true;
			s->rays.wallhit_x[ray_id] = next_checkx;
			s->rays.wallhit_y[ray_id] = next_checky;
			s->rays.distance_sq[ray_id] = lev_distance_sq(s->p.x, s->p.y, next_checkx, next_checky); 
			return;	
		} else {
			next_checkx += xstep;
			next_checky += ystep;
		}
	}
}

void detect_vertical_intercept(struct state *s, int ray_id, float ray_sin, float ray_cos)
{
	float angle = s->rays.angle[ray_id];
	bool is_facing_down = (angle > 0 && angle < PI);
	bool is_facing_up = !is_facing_down;
	bool is_facing_right = (angle < PI * 0.5 || angle > 1.5 * PI);
	bool is_facing_left = !is_facing_right;

	float xintercept = floor(s->p.x / TILE_SIZE) * TILE_SIZE;
	xintercept += (is_facing_right) ? TILE_SIZE : 0;

	float yintercept = s->p.y + (xintercept - s->p.x) * (ray_sin / ray_cos);

	float xstep = TILE_SIZE;
	xstep *= (is_facing_left) ? -1 : 1;

	float ystep = TILE_SIZE * (ray_sin / ray_cos);
	ystep *= (is_facing_down && ystep < 0) ? -1 : 1;
	ystep *= (is_facing_up && ystep > 0) ? -1 : 1;

	float next_checkx = xintercept;
	float next_checky = yintercept;
	
	while (1) {
		int map_y = (int)(next_checky / TILE_SIZE);
		int map_x = (int)((next_checkx + (is_facing_left ? -1 : 0))) / TILE_SIZE;

		if (map_x < 0 || map_x >= MAP_COLS || map_y < 0 || map_y >= MAP_ROWS || map[map_y][map_x] == 1) {
			float distance_sq = lev_distance_sq(s->p.x, s->p.y, next_checkx, next_checky);
			s->rays.is_hit_wall[ray_id] = true;
			if (distance_sq < s->rays.distance_sq[ray_id]) {
				s->rays.is_hit_vertwall[ray_id] = true;
				s->rays.wallhit_x[ray_id] = next_checkx;
				s->rays.wallhit_y[ray_id] = next_checky;	
				s->rays.distance_sq[ray_id] = distance_sq;
			}
			return;
		} else {
			next_checkx += xstep;
			next_checky += ystep;
		}
	}
}

static float sanitize_angle(float angle)
{
	angle = fmodf(angle, (float)(2 * PI));
	return (angle < 0) ? (2 * PI) + angle : angle;
}

void update_rays(struct state *s)
{
	float base_angle = s->p.rotation_angle - (FOV_ANGLE / 2);
	float step = FOV_ANGLE / NUM_RAYS;
	for (int i = 0; i < NUM_RAYS; i++) {
		s->rays.angle[i] = sanitize_angle(base_angle + (step * i));

		float ray_sin = sin(s->rays.angle[i]);
		float ray_cos = cos(s->rays.angle[i]);
		s->rays.distance_sq[i] = 999999.0f;
		s->rays.is_hit_wall[i] = false;
		s->rays.is_hit_vertwall[i] = false;

		if (fabs(ray_sin) >= 0.0001f)
			detect_horizontal_intercept(s, i, ray_sin, ray_cos);	
		if (fabs(ray_cos) >= 0.0001f)
			detect_vertical_intercept(s, i, ray_sin, ray_cos);
	}
}

void render_map(struct state *s) 
{
	lev_draw_rect(s->pixels, RES_WIDTH, RES_HEIGHT, 0, 0, TILE_SIZE * MAP_COLS * MINI_MAP_SCALE, TILE_SIZE * MAP_ROWS * MINI_MAP_SCALE, 0xffffffff);

	for (int y = 0; y < MAP_ROWS; y++) {
		for (int x = 0; x < MAP_COLS; x++) {
			if (map[y][x] != 0) {
				int drawy = y * TILE_SIZE * MINI_MAP_SCALE;
				int drawx = x * TILE_SIZE * MINI_MAP_SCALE; 
				int nexty = (y + 1) * TILE_SIZE * MINI_MAP_SCALE; 
				int nextx = (x + 1) * TILE_SIZE * MINI_MAP_SCALE; 
				int draw_w = nextx - drawx;
				int draw_h = nexty - drawy; 
				lev_draw_rect(s->pixels, RES_WIDTH, RES_HEIGHT, drawx, drawy, draw_w, draw_h, 0x000000ff);
			}
		}
	}
}

void render_rays(struct state *s) 
{
	int ray_x, ray_y;
	for (int i = 0; i < NUM_RAYS; i++) {
		if (s->rays.is_hit_wall[i] == false) {
			ray_x = (s->p.x * MINI_MAP_SCALE) + cos(s->rays.angle[i]) * 500;
			ray_y = (s->p.y * MINI_MAP_SCALE) + sin(s->rays.angle[i]) * 500;	
		} else {
			ray_x = (int)s->rays.wallhit_x[i] * MINI_MAP_SCALE;
			ray_y = (int)s->rays.wallhit_y[i] * MINI_MAP_SCALE;
		}
		lev_draw_line(s->pixels, RES_WIDTH, RES_HEIGHT, s->p.x * MINI_MAP_SCALE, s->p.y * MINI_MAP_SCALE, 
				ray_x, ray_y, 0xffff00ff); 
	}
}

void render_player(struct state *s)
{
	int mini_px = s->p.x * MINI_MAP_SCALE;
	int mini_py = s->p.y * MINI_MAP_SCALE;
	int mini_pr = s->p.r * MINI_MAP_SCALE;
	lev_draw_circle(s->pixels, RES_WIDTH, RES_HEIGHT, 
			mini_px, mini_py, mini_pr, 0xff0000ff);
	lev_draw_line(s->pixels, RES_WIDTH, RES_HEIGHT, 
			mini_px, mini_py, mini_px + (cos(s->p.rotation_angle) * 40 * MINI_MAP_SCALE), mini_py + (sin(s->p.rotation_angle) * 40 * MINI_MAP_SCALE),
			0xee0000ff);
}

void render_walls(struct state *s)
{
	float projection_distance = (RES_WIDTH / 2) / tan(FOV_ANGLE/2) ;
	for (int i = 0; i < NUM_RAYS; i++) {
		float a = s->rays.angle[i];
		float distance_to_wall = sqrt(s->rays.distance_sq[i]);
		distance_to_wall *= cos(a - s->p.rotation_angle); // distortion adjustment
		float projected_wall_height = (TILE_SIZE / distance_to_wall) * projection_distance;
		
		int wall_top = (RES_HEIGHT / 2) - ((int)projected_wall_height / 2);
		int wall_bottom = (RES_HEIGHT / 2) + ((int)projected_wall_height / 2);
		uint32_t color = (s->rays.is_hit_vertwall[i]) ? 0xffffffff : 0xddddddff;
		lev_draw_line(s->pixels, RES_WIDTH, RES_HEIGHT, i, wall_top, i, wall_bottom, color);
	}
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	struct state s = {0};
	s.p.x = MAP_WIDTH / 2;
	s.p.y = MAP_HEIGHT / 2;
	s.p.r = 10;
	s.p.rotation_angle = PI / 2;
	s.p.walk_speed = TILE_SIZE * 3;
	s.p.turn_speed = TILE_SIZE * 3 * (PI / 180);

	s.pixels = malloc(RES_WIDTH * RES_HEIGHT * sizeof(uint32_t)); 
	if (!s.pixels)
		goto cleanup;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		goto cleanup;

	if (sdl_setup(&s) < 0)
		goto cleanup;
	
	main_loop(&s);

cleanup:
	sdl_cleanup(&s);
	if (s.pixels)
		free(s.pixels);
	return 0;
}

static int sdl_setup(struct state *s)
{
	s->window = SDL_CreateWindow(
			"Raycasting demo",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			WIN_WIDTH,
			WIN_HEIGHT,
			SDL_WINDOW_SHOWN
			);

	if (!s->window) 
		return -1;

	s->renderer = SDL_CreateRenderer(s->window, -1, SDL_RENDERER_ACCELERATED);
	if (!s->renderer) 
		return -1;

	s->texture = SDL_CreateTexture(s->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
			RES_WIDTH, RES_HEIGHT);
	if (!s->texture)
		return -1;

	return 0;
}

static void main_loop(struct state *s)
{
	bool is_running = true;
	SDL_Event event;

	while (is_running) {
		float time_to_wait = (1000 / FPS) - (SDL_GetTicks() - s->ticks_lastframe);
		if (time_to_wait > 0 && time_to_wait <= (1000 / FPS)) {
			SDL_Delay(time_to_wait);
		}

		float dt = (SDL_GetTicks() - s->ticks_lastframe) / 1000.0f;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				is_running = false;

			} else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE)  
					is_running = false;
				if (event.key.keysym.sym == SDLK_UP) 
					s->p.walk_direction = 1;
				if (event.key.keysym.sym == SDLK_DOWN) 
					s->p.walk_direction = -1;
				if (event.key.keysym.sym == SDLK_RIGHT)
					s->p.turn_direction = 1;
				if (event.key.keysym.sym == SDLK_LEFT)
					s->p.turn_direction = -1;
			} else if (event.type == SDL_KEYUP) {
				if (event.key.keysym.sym == SDLK_UP) 
					s->p.walk_direction = 0; 
				if (event.key.keysym.sym == SDLK_DOWN) 
					s->p.walk_direction = 0;
				if (event.key.keysym.sym == SDLK_RIGHT)
					s->p.turn_direction = 0;
				if (event.key.keysym.sym == SDLK_LEFT)
					s->p.turn_direction = 0;
			}
		}

		move_player(s, dt);
		update_rays(s);

		lev_draw_fill(s->pixels, RES_WIDTH, RES_HEIGHT, 0x808080ff);
		lev_draw_rect(s->pixels, RES_WIDTH, RES_HEIGHT, 0, 0, RES_WIDTH, RES_HEIGHT / 2, 0x5d5d5dff);
		render_walls(s);
		render_map(s);
		render_rays(s);
		render_player(s);
		SDL_UpdateTexture(s->texture, NULL, s->pixels, RES_WIDTH * sizeof(uint32_t));
		SDL_RenderCopy(s->renderer, s->texture, NULL, NULL);
		SDL_RenderPresent(s->renderer);

		s->ticks_lastframe = SDL_GetTicks();
	}
}

static void sdl_cleanup(struct state *s)
{
	SDL_DestroyTexture(s->texture);
	SDL_DestroyRenderer(s->renderer);
	SDL_DestroyWindow(s->window);
	SDL_Quit();
}
