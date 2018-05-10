#include <darnit/darnit.h>
#include <math.h>
#include "main.h"
#include "ailib.h"
//#include "network/protocol.h"
//#include "server/server.h"
#include "util.h"
//#include "block.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>


int movableInit() {
	s->movable.movable = NULL;
	s->movable.coll_buf = NULL;
	s->movable.bbox = NULL;
	s->movable.movables = 0;

	return 0;
}


void gcenter_calc(int x, int y, int *gx, int *gy) {
	x = (x - GRAVITY_CENTER_X) * GRAVITY_SCALE;
	y = (y - GRAVITY_CENTER_Y) * GRAVITY_SCALE;

	if (x * x + y * y > GRAVITY_CAP * GRAVITY_CAP) {
		*gx = -(x * GRAVITY_CAP / sqrt(x * x + y * y));
		*gy = -(y * GRAVITY_CAP / sqrt(x * x + y * y));
	} else
		*gx = x, *gy = y;

}


static int _lookup_movable_player_id(int id) {
	const char *playerid_str;
	int i;

	for (i = 0; i < s->movable.movables; i++) {
		if (!(playerid_str = s->active_level->object[s->movable.movable[i].id].ref, "block_id"))
			return 0;
		if (atoi(playerid_str) == id)
			return i;
	}

	return 0;
}


void movableSpawn() {
	int i, h_x, h_y, h_w, h_h, team1 = 0, team2 = 16;

	for (i = 0; i < s->movable.movables; i++) {
		if (s->movable.movable[i].prevent_respawn)
			continue;
		d_sprite_direction_set(s->movable.movable[i].sprite, 0);
		s->movable.movable[i].ai = ailib_get(d_map_prop(s->active_level->object[i].ref, "ai"));
		s->movable.movable[i].x = s->active_level->object[i].x * 1000 * s->active_level->layer->tile_w;
		s->movable.movable[i].y = s->active_level->object[i].y * 1000 * s->active_level->layer->tile_h;
		s->movable.movable[i].l = s->active_level->object[i].l;
		s->movable.movable[i].direction = 0;
		//s->movable.movable[i].hp = 1;
		s->movable.movable[i].angle = 0;
		s->movable.movable[i].gravity_effect = 0;
		s->movable.movable[i].x_velocity = 0;
		s->movable.movable[i].y_velocity = 1;
		s->movable.movable[i].id = i;
		d_sprite_hitbox(s->movable.movable[i].sprite, &h_x, &h_y, &h_w, &h_h);
		d_bbox_add(s->movable.bbox, s->movable.movable[i].x / 1000 + h_x, s->movable.movable[i].x / 1000 + h_x, h_w, h_h);
		if (s->movable.movable[i].ai)
			s->movable.movable[i].ai(s, &s->movable.movable[i], MOVABLE_MSG_INIT);

	}

	for (i = 0; i < PLAYER_CAP; i++) {
		if (!s->player[i].active)
			s->player[i].movable = 31;
		else if (!s->player[i].team) {
			s->player[i].movable = team1++;
		} else {
			s->player[i].movable = team2++;
		}
	}

	if (s->is_host) {
		//for (i = team1; i < 16; i++)
		//	s->movable.movable[_lookup_movable_player_id(i)].hp = 0;
		//for (i = team2; i < 32; i++)
		//	s->movable.movable[_lookup_movable_player_id(i)].hp = 0;
	}

	return;
}


int movableTileCollision(MOVABLE_ENTRY *entry, int off_x, int off_y) {
	int box_x, box_y, box_w, box_h, w;

	d_sprite_hitbox(entry->sprite, &box_x, &box_y, &box_w, &box_h);
	box_x += (entry->x / 1000);
	box_y += (entry->y / 1000);
	box_x += (off_x < 0) ? -1 : (box_w + 1 * (off_x == 0 ? -1 : 1));
	box_y += (off_y < 0) ? -1 : (box_h + 1 * (off_y == 0 ? -1 : 1));
	off_x += (off_x == -2) ? 1 : 0;
	off_y += (off_y == -2) ? 1 : 0;

	box_x /= s->active_level->layer[entry->l].tile_w;
	box_y /= s->active_level->layer[entry->l].tile_h;
	w = s->active_level->layer[entry->l].tilemap->w;
	return s->active_level->layer[entry->l].tilemap->data[box_x + box_y * w];
}


int movableLoad() {
	int i;
	MOVABLE_ENTRY *entry;

	movableKillEmAll();

	for (i = 0; i < s->movable.movables; i++)
		d_sprite_free(s->movable.movable[i].sprite);

	if (!(entry = realloc(s->movable.movable, sizeof(MOVABLE_ENTRY) * s->active_level->objects))) {
		free(s->movable.movable);
		free(s->movable.coll_buf);
		free(s->movable.ai_coll_buf);
		d_bbox_free(s->movable.bbox);
		movableInit();
		return -1;
	}

	s->movable.movable = entry;
	s->movable.movables = s->active_level->objects;
	s->movable.coll_buf = malloc(sizeof(int) * s->active_level->objects);
	s->movable.ai_coll_buf = malloc(sizeof(int) * s->active_level->objects);
	s->movable.bbox = d_bbox_new(s->movable.movables);
	d_bbox_set_indexkey(s->movable.bbox);

	for (i = 0; i < s->movable.movables; i++) {
		s->movable.movable[i].sprite = d_sprite_load(util_binrel_path(d_map_prop(s->active_level->object[i].ref, "sprite")), 0, DARNIT_PFORMAT_RGB5A1);
		d_sprite_animate_start(s->movable.movable[i].sprite);
		s->movable.movable[i].prevent_respawn = 0;

	}
	
	movableSpawn();
	movableLoop();

	return 0;
}


/* NOTE: An award of one strawberry goes to whoever can guess who suggested this as the function suffix */
void movableKillEmAll() {
	int i;

	for (i = 0; i < s->movable.movables; i++) {
		if (!s->movable.movable[i].ai)
			continue;
		if (s->movable.movable[i].hp <= 0)
			continue;
		s->movable.movable[i].ai(s, &s->movable.movable[i], MOVABLE_MSG_DESTROY);
	}

	d_bbox_clear(s->movable.bbox);

	return;
}


void movableRespawn() {
	movableKillEmAll();
	movableSpawn();
	movableLoop();

	return;
}


void movableMoveDo(DARNIT_MAP_LAYER *layer, int *pos, int *delta, int *vel, int space, int col, int hit_off, int vel_r, int i, int i_b) {
	int u, t, tile_w, tile_h, map_w, i_2;
	unsigned int *map_d;
	tile_w = layer->tile_w;
	tile_h = layer->tile_h;
	map_w = layer->tilemap->w;
	map_d = layer->tilemap->data;

	if (!(*delta))
		return;
	u = (*pos) / 1000 + hit_off;
	t = u + (((*delta) > 0) ? 1 : -1);

	if (i < 0) {
		u /= tile_h;
		t /= tile_h;
	} else {
		u /= tile_w;
		t /= tile_w;
	}

	if (u == t) {
		(*pos) += space;
		(*delta) -= space;
		return;
	}

	i_2 = (i < 0) ? t * map_w + (abs(i) + i_b) / tile_w : ((i + i_b) / tile_h) * map_w + t;
	i = (i < 0) ? t * map_w + abs(i) / tile_w : (i / tile_h) * map_w + t;
	
	if (map_d[i] & col || map_d[i_2] & col) {
		(*vel) = vel_r;
		(*delta) = 0;
		return;
	}

	(*pos) += space;
	(*delta) -= space;
	return;
}


int movableHackTest(MOVABLE_ENTRY *entry) {
	return !((movableTileCollision(entry, -1, 1) & movableTileCollision(entry, 1, 1)) & COLLISION_TOP);
}


int movableGravity(MOVABLE_ENTRY *entry) {
	//int gravity, hack;
	int delta_x, delta_y, r, p, gravity_x, gravity_y;
	int hit_x, hit_y, hit_w, hit_h;

	DARNIT_MAP_LAYER *layer = &(s->active_level->layer[entry->l]);

	d_sprite_hitbox(entry->sprite, &hit_x, &hit_y, &hit_w, &hit_h);
	hit_w--;
	hit_h--;

	if (entry->gravity_effect) {
		gcenter_calc(entry->x / 1000, entry->y / 1000, &gravity_x, &gravity_y);
		entry->x_gravity += gravity_x * d_last_frame_time();
		entry->y_gravity += gravity_y * d_last_frame_time();
		if ((entry->x_gravity * entry->x_gravity + entry->y_gravity + entry->y_gravity) > MOV_TERMINAL_VELOCITY * MOV_TERMINAL_VELOCITY) {
			gravity_x = MOV_TERMINAL_VELOCITY * entry->x_gravity / sqrt(entry->x_gravity * entry->x_gravity + entry->y_gravity + entry->y_gravity);
			gravity_y = MOV_TERMINAL_VELOCITY * entry->y_gravity / sqrt(entry->x_gravity * entry->x_gravity + entry->y_gravity + entry->y_gravity);
		}


		/* Y-axis */
		delta_y = (entry->y_gravity * d_last_frame_time());

		/* X-axis */
		delta_x = entry->x_gravity * d_last_frame_time();
		/* TODO: STUB */

		p = delta_x * 1000 / (delta_y ? delta_y : 1);

		while (delta_x || delta_y) {
			if (delta_x && ((!delta_y || delta_x * 1000 / (delta_y ? delta_y : 1) > p))) {
				r = entry->x % 1000;
				if (r + delta_x < 1000 && r + delta_x >= 0) {
					entry->x += delta_x;
					delta_x = 0;
					continue;
				}

				if (delta_x > 0) {
					r = 1000 - r;
					movableMoveDo(layer, &entry->x, &delta_x, &entry->x_velocity, r, COLLISION_LEFT, hit_x + hit_w, 0, entry->y / 1000 + hit_y, hit_h);
				} else { 
					if (!r)
						r = 1000;
					r *= -1;
					movableMoveDo(layer, &entry->x, &delta_x, &entry->x_velocity, r, COLLISION_RIGHT, hit_x, 0, entry->y / 1000 + hit_y, hit_h);
				}
			} else {
				r = entry->y % 1000;
				if (r + delta_y < 1000 && r + delta_y >= 0) {
					entry->y += delta_y;
					delta_y = 0;
					continue;
				}
			
				if (delta_y > 0) {
					r = 1000 - r;
					movableMoveDo(layer, &entry->y, &delta_y, &entry->y_velocity, r, COLLISION_TOP, hit_y + hit_h, 0, entry->x / -1000 - hit_x, hit_w);
				} else {
					if (!r)
						r = 1000;
					r *= -1;
				}
			} 

		}
	}

	#if 0
	while (delta_x || delta_y) {
		if (delta_x && (!delta_y || delta_x * 1000 / (delta_y ? delta_y : 1) > p)) {
			r = entry->x % 1000;
			if (r + delta_x < 1000 && r + delta_x >= 0) {
				entry->x += delta_x;
				delta_x = 0;
				continue;
			}

			if (delta_x > 0) {
				r = 1000 - r;
				/*if (movableMoveDoTestTowers(entry->x, delta_x, entry->x_velocity, COLLISION_LEFT, hit_x + hit_w, entry->y / 1000 + hit_y, hit_h)) {
					entry->x_velocity = 0, delta_x = 0;
					continue;
				}*/

				movableMoveDo(layer, &entry->x, &delta_x, &entry->x_velocity, r, COLLISION_LEFT, hit_x + hit_w, 0, entry->y / 1000 + hit_y, hit_h);
			} else {
				if (!r)
					r = 1000;
				r *= -1;
				/*if (movableMoveDoTestTowers(entry->x, delta_x, entry->x_velocity, COLLISION_RIGHT, hit_x, entry->y / 1000 + hit_y, hit_h)) {
					entry->x_velocity = 0, delta_x = 0;
					continue;
				}*/

				movableMoveDo(layer, &entry->x, &delta_x, &entry->x_velocity, r, COLLISION_RIGHT, hit_x, 0, entry->y / 1000 + hit_y, hit_h);
			}
		} else {	/* delta_y måste vara != 0 */
			r = entry->y % 1000;
			if (r + delta_y < 1000 && r + delta_y >= 0) {
				entry->y += delta_y;
				delta_y = 0;
				if (!hack && !movableHackTest(entry)) {
					entry->y_velocity = hack;
				}
				continue;
			} 

			if (delta_y > 0) {
				r = 1000 - r;
				/*if (movableMoveDoTestTowers(entry->y, delta_y, entry->y_velocity, COLLISION_TOP, hit_y + hit_h, entry->x / -1000 - hit_x, hit_w)) {
					entry->y_velocity = 0, delta_y = 0;
					continue;
				}*/
				movableMoveDo(layer, &entry->y, &delta_y, &entry->y_velocity, r, COLLISION_TOP, hit_y + hit_h, 0, entry->x / -1000 - hit_x, hit_w);
			} else {
				if (!r)
					r = 1000;
				r *= -1;
				/*if (movableMoveDoTestTowers(entry->y, delta_y, entry->y_velocity, COLLISION_BOTTOM, hit_y, entry->x / -1000 - hit_x, hit_w)) {
					entry->y_velocity = -1, delta_y = 0;
					continue;
				}*/
				movableMoveDo(layer, &entry->y, &delta_y, &entry->y_velocity, r, COLLISION_BOTTOM, hit_y, -1, entry->x / -1000 - hit_x, hit_w);
			}
		}
	}
	#endif

	return -1462573849;
}


void movableLoop() {
	int i, j, h_x, h_y, h_w, h_h, players_active = 0, winning_player = -1;
	bool master = s->is_host;

	//res = d_bbox_test(s->movable.bbox, 0, 0, INT_MAX, INT_MAX, s->movable.coll_buf, ~0);

	for (j = 0; j < s->movable.movables; j++) {
		i = j;
		if (!s->movable.movable[i].hp)
			continue;
		if (master) {
			if (_get_player_id(&s->movable.movable[i]) >= 0)
				players_active++, winning_player = _get_player_id(&s->movable.movable[i]);
			if (s->movable.movable[i].ai)
				s->movable.movable[i].ai(s, &s->movable.movable[i], MOVABLE_MSG_LOOP);
			movableGravity(&s->movable.movable[i]);
		}


		if (s->movable.movable[i].direction != d_sprite_direction_get(s->movable.movable[i].sprite))
			d_sprite_direction_set(s->movable.movable[i].sprite, s->movable.movable[i].direction);
		d_sprite_move(s->movable.movable[i].sprite, s->movable.movable[i].x / 1000, s->movable.movable[i].y / 1000);
		d_sprite_hitbox(s->movable.movable[i].sprite, &h_x, &h_y, &h_w, &h_h);
//		d_sprite_rotate(s->movable.movable[i].sprite, s->movable.movable[i].angle>1800?s->movable.movable[i].angle:3600-s->movable.movable[i].angle);
		d_sprite_rotate(s->movable.movable[i].sprite, s->movable.movable[i].angle);
		d_bbox_move(s->movable.bbox, i, s->movable.movable[i].x / 1000 + h_x, s->movable.movable[i].y / 1000 + h_y);
		d_bbox_resize(s->movable.bbox, i, h_w, h_h);
		if (s->movable.movable[i].hp <= 0) {
			d_bbox_delete(s->movable.bbox, i);
			if (s->movable.movable[i].ai)
				s->movable.movable[i].ai(s, &s->movable.movable[i], MOVABLE_MSG_DESTROY);
			/* TODO: Make it play some sound effect here */
		}
	}

	if (players_active <= 1 && master) {
		//fprintf(stderr, "win condition\n");
		//server_announce_winner(winning_player);
	}

}


void movableKillObject(int id) {
	d_bbox_delete(s->movable.bbox, id);
	if (s->movable.movable[id].ai)
		s->movable.movable[id].ai(s, &s->movable.movable[id], MOVABLE_MSG_DESTROY);
	/* TODO: Should broadcast this */
}


void movableFreezeSprites(int freeze) {
	int i;

	for (i = 0; i < s->movable.movables; i++)
		(!freeze ? d_sprite_animate_start : d_sprite_animate_pause)(s->movable.movable[i].sprite);
	return;
}


void movableLoopRender(int layer) {
	int i;

//	res = d_bbox_test(s->movable.bbox, s->camera.x - 128, s->camera.y - 128, d_platform_get().screen_w + 256, d_platform_get().screen_h + 256, s->movable.coll_buf, ~0);

	for (i = 0; i < s->movable.movables; i++) {
		if (s->movable.movable[i].l != layer)
			continue;
		if (!s->movable.movable[i].hp) {
			continue;
		}
		d_sprite_draw(s->movable.movable[i].sprite);
	}
}


