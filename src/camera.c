#include "main.h"
#include "camera.h"


void camera_work() {
	int x, y;

	if (s->camera.follow < 0)
		return;
	x = s->movable.movable[s->camera.follow].x / 1000;
	y = s->movable.movable[s->camera.follow].y / 1000;

	x -= d_platform_get().screen_w / 2;
	y -= d_platform_get().screen_h / 2;

	if (x + d_platform_get().screen_w > s->active_level->layer->tilemap->w * s->active_level->layer->tile_w)
		x = s->active_level->layer->tilemap->w * s->active_level->layer->tile_w - d_platform_get().screen_w;
	if (y + d_platform_get().screen_h > s->active_level->layer->tilemap->h * s->active_level->layer->tile_h)
		y = s->active_level->layer->tilemap->h * s->active_level->layer->tile_h - d_platform_get().screen_h;
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	s->camera.x = x;
	s->camera.y = y;
}
