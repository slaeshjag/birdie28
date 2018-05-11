#include "ailib.h"
#include "movable.h"
//#include "ingame.h"
#include "main.h"
//#include "block.h"
#include "network/protocol.h"
#include "trigonometry.h"
//#include "blocklogic.h"
//#include "soundeffects.h"
#include "effect.h"
#include "server/server.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define DEATH_VELOCITY 400
#define	EFFECT_RANGE 200

enum AI_APPLE_STATE {
	AI_APPLE_STATE_HIDING,
	AI_APPLE_STATE_FALLING,
};


enum AI_APPLE_BULLET_STATE {
	AI_APPLE_BULLET_STATE_IDLE,
	AI_APPLE_BULLET_STATE_FLYING,
};


enum AI_PLAYER_STATE {
	AI_PLAYER_WALKING,
	AI_PLAYER_LEAPING,
};


struct AppleState {
	int			last_second;
	enum AI_APPLE_STATE 	state;
};


struct AppleBulletState {
	enum AI_APPLE_BULLET_STATE state;
	int			owner;
};


struct AIPlayerState {
	enum AI_PLAYER_STATE	state;
	int			apple[4];
	int			selected_apple;
	time_t			stunned_time;
	time_t			fucked_controller_time;
};


static void _trigger_effect(int x, int y, int player, int effect) {
	int i, dx, dy;

	Packet pack;

	pack.type = PACKET_TYPE_PARTICLE;
	pack.size = sizeof(PacketParticle);

	pack.particle.x = x;
	pack.particle.y = y;
	pack.particle.effect_type = effect;


	protocol_send_packet(server_sock, &pack);

	for (i = 0; i < PLAYER_CAP; i++) {
		if (i == player)
			continue;
		if (!s->player[i].active)
			continue;
		dx = s->movable.movable[s->player[i].movable].x / 1000;
		dy = s->movable.movable[s->player[i].movable].y / 1000;

		dx -= x;
		dy -= y;

		if (dx * dx + dy * dy > EFFECT_RANGE * EFFECT_RANGE)
			continue;

		printf("Hit player %i\n", i);

		switch(effect) {
			case EFFECT_STUN:
				s->player[i].stunned = true;
				break;
			case EFFECT_DROP:
				for(int j = 0; j < 4; j++) {
					s->player[i].apple[j] -= rand() % 2;
					if(s->player[i].apple[j] < 0)
						s->player[i].apple[j] = 0;
				}
				break;
			case EFFECT_SLAPPED_AROUND:
				s->movable.movable[s->player[i].movable].x_velocity = (signed) ((rand() % 10000)) - 5000;
				s->movable.movable[s->player[i].movable].y_velocity = (signed) ((rand() % 10000)) - 5000;
				break;
			case EFFECT_FUCKED_CONTROLS:
				s->player[i].fucked_controls = true;
				break;
			default:
				printf("Unknown effect %i\n", effect);
		}
	}
}


static int _push_apple_count(int apple[4], int player, int selected) {
	Packet pack;

	pack.type = PACKET_TYPE_APPLE_COUNT;
	pack.size = sizeof(pack.apple_count);

	pack.apple_count.apple[0] = apple[0];
	pack.apple_count.apple[1] = apple[1];
	pack.apple_count.apple[2] = apple[2];
	pack.apple_count.apple[3] = apple[3];
	pack.apple_count.selected = selected;
	pack.apple_count.player = player;

	return protocol_send_packet(server_sock, &pack);
}


int _get_block_from_entry(MOVABLE_ENTRY *self) {
	const char *playerid_str = d_map_prop(s->active_level->object[self->id].ref, "player_id");
	int i;

	for (i = 0; i < s->active_level->objects; i++)
		if (!strcmp(playerid_str, d_map_prop(s->active_level->object[i].ref, "block_id")))
			return i;
	fprintf(stderr, "bad block\n");
	return -1;
}


int _get_player_id(MOVABLE_ENTRY *self) {
	/* On a scale of 1 to italy, how inefficient is this? */
	const char *playerid_str;
	int movable_id, i;
	if (!(playerid_str = d_map_prop(s->active_level->object[self->id].ref, "player_id"))) {
		return -1;
	}

	//fprintf(stderr, "%s, %i\n", playerid_str, self->id);
	movable_id = atoi(playerid_str);
	//return movable_id;
	for (i = 0; i < PLAYER_CAP; i++) {
		if (!s->player[i].active)
			continue;
		//fprintf(stderr, "active %i, %i, %i\n", i, s->player[i].movable, movable_id);
		if (s->player[i].movable == movable_id)
			return i;
	}
	return -1;
}


static int _player_direction(MOVABLE_ENTRY *self) {
	int player_id;

	player_id = _get_player_id(self);
	if (player_id < 0 || player_id >= PLAYER_CAP)
		return 0;
	//if (s->player[player_id].holding->direction)
	//	return (!s->player[player_id].last_walk_direction) + 4;
	
	return ((!s->player[player_id].last_walk_direction));
}


static int _player_fix_hitbox(MOVABLE_ENTRY *self) {
	int dir, box1_x, box2_x, box1_y, box2_y, box1_w, box2_w, box1_h, box2_h, diff;
	if ((movableTileCollision(self, 0, 0) & COLLISION_KILL) || (movableTileCollision(self, 0, -2) & COLLISION_KILL) || (movableTileCollision(self, -2, 0) & COLLISION_KILL) || (movableTileCollision(self, -2, -2) & COLLISION_KILL));
	// TODO: Do something with this //
	
	dir = _player_direction(self);
	if (dir == self->direction)
		return 0;
	d_sprite_hitbox(self->sprite, &box1_x, &box1_y, &box1_w, &box1_h);
	d_sprite_direction_set(self->sprite, dir);
	d_sprite_hitbox(self->sprite, &box2_x, &box2_y, &box2_w, &box2_h);
	diff = (((box2_y + box2_h) - (box1_y + box1_h)));
	self->y -= (diff * 1000);

	diff = (box2_x - box1_x);
	if (movableTileCollision(self, -1, -1) & COLLISION_RIGHT)
		self->x += abs(diff * 1000);
	else {
		if (movableTileCollision(self, 1, 1) & COLLISION_LEFT)
			self->x -= abs(diff * 2000);
	}

	return 1;
}


static void _die(MOVABLE_ENTRY *self, int player_id) {
	Packet pack;
	
	
	self->x_velocity = self->y_velocity = 0;
	
	self->x = s->active_level->object[self->id].x*1000*24;
	self->y = s->active_level->object[self->id].y*1000*24;
					
	//s->player[player_id].holding->direction = block_spawn();
}


void ai_apple_bullet(void *dummy, void *entry, MOVABLE_MSG msg) {
	MOVABLE_ENTRY *self = entry;
	struct AppleBulletState *state = self->mystery_pointer;
	struct AILibFireMSG *fm = dummy;

	if (!s->is_host)
		return;
	switch (msg) {
		case MOVABLE_MSG_INIT:
			self->flag = 1;
			self->hp = 100;
			self->gravity_effect = 0;
			self->direction = 0;
			self->y_velocity = 0;
			self->mystery_pointer = calloc(sizeof(*state), 1);
			state = self->mystery_pointer;
			state->owner = atoi(d_map_prop(s->active_level->object[self->id].ref, "owner"));
			s->player[state->owner].bullet_movable = self->id;
			break;
		case MOVABLE_MSG_LOOP:
			if (self->flag) {
				self->flag = 0;
				return;
			}

			if (state->state == AI_APPLE_BULLET_STATE_IDLE) {
				self->gravity_effect = 0;
				self->direction = 0;
				self->x_gravity = self->y_gravity = 0;
				self->x_velocity = self->y_velocity = 0;
			} else {
				self->gravity_effect = 1;
				//printf("Bullet at %i %i\n", self->x / 1000, self->y / 1000);
				if (self->gravity_blocked || self->movement_blocked)  {
					state->state = AI_APPLE_BULLET_STATE_IDLE;
					printf("splat\n");
					_trigger_effect(self->x/1000, self->y/1000, _get_player_id(self), self->direction - 1);
				}
			}
			
			break;
		case MOVABLE_MSG_REQUEST_FIRE:
			if (state->state == AI_APPLE_BULLET_STATE_FLYING)
				break;
			s->movable.movable[s->player[state->owner].movable].ai(dummy, &s->movable.movable[s->player[state->owner].movable], MOVABLE_MSG_CHECK_APPLES);
			break;
		case MOVABLE_MSG_FIRE:
			//printf("Fire!\n");
			self->direction = fm->direction;
			self->x_velocity = fm->xvec;
			self->y_velocity = fm->yvec;
			
			self->x = s->movable.movable[s->player[state->owner].movable].x + (util_sprite_xoff(s->movable.movable[state->owner].sprite) + util_sprite_width(s->movable.movable[state->owner].sprite)/2 + d_sprite_width(self->sprite)/2)*1000;
			self->y = s->movable.movable[s->player[state->owner].movable].y + (util_sprite_yoff(s->movable.movable[state->owner].sprite) + util_sprite_height(s->movable.movable[state->owner].sprite)/2 + d_sprite_height(self->sprite)/2)*1000;
			//printf("Fired at %i %i\n", self->x / 1000, self->y / 1000);
			self->gravity_effect = 0;
			state->state = AI_APPLE_BULLET_STATE_FLYING;
			break;
		case MOVABLE_MSG_DESTROY:
			free(self->mystery_pointer);
			break;
		default:
			return;
	}
}


void ai_apple(void *dummy, void *entry, MOVABLE_MSG msg) {
	MOVABLE_ENTRY *self = entry;
	struct AppleState *state = self->mystery_pointer;

	if (!s->is_host)
		return;
	switch (msg) {
		case MOVABLE_MSG_INIT:
			self->flag = 1;
			self->hp = 100;
			self->gravity_effect = 0;
			self->direction = 0;
			self->y_velocity = 0;
			self->mystery_pointer = calloc(sizeof(*state), 1);
			break;
		case MOVABLE_MSG_LOOP:
			if (self->flag) {
				self->flag = 0;
				return;
			}
		
			if (state->state == AI_APPLE_STATE_HIDING) {
				if (d_time_get()/1000 != state->last_second) {
					state->last_second = d_time_get()/1000;
					if (!(rand() % 10)) {
						state->state = AI_APPLE_STATE_FALLING;
						self->direction = 1;
						self->gravity_effect = 1;
					}
				}
			}

			self->direction = ((d_time_get() / 2000) & 1);
			break;
		case MOVABLE_MSG_DESTROY:
			free(self->mystery_pointer);
			break;
		default:
			break;
	}
}


void ai_player(void *dummy, void *entry, MOVABLE_MSG msg) {
	MOVABLE_ENTRY *self = entry;
	struct AIPlayerState *state = self->mystery_pointer;
	struct AILibFireMSG *fm = dummy;
	int player_id;

	if (!s->is_host) {
		fprintf(stderr, "Not host\n");
		return;
	}
	switch (msg) {
		case MOVABLE_MSG_INIT:
			self->flag = 1;
			self->hp = self->hp_max = 400;
			self->gravity_effect = 1;
			self->mystery_pointer = calloc(sizeof(*state), 1);
			state = self->mystery_pointer;
			state->apple[0] = 5;
			state->apple[1] = 1;
			state->apple[2] = 2;
			state->apple[3] = 3;
			_push_apple_count(state->apple, _get_player_id(self), state->selected_apple);
			//if (player_id >= PLAYER_CAP)	// TODO: replace PLAYER_CAP with actual number of connected players //
			//	self->hp = 0;
			//if (!server_player_is_present(player_id))
			//	self->hp = 0;
			break;
		case MOVABLE_MSG_LOOP:
			player_id = _get_player_id(self);
			//if (_player_fix_hitbox(self))
			//	break;
			if (self->flag) {
				if (player_id >= 0) {
					s->player[player_id].last_walk_direction = 0;
					//s->player[player_id].holding = &s->movable.movable[_get_block_from_entry(self)];
					s->player[player_id].pulling = 0;
				}
				self->flag = 0;
				if (player_id < 0 || !s->player[player_id].active)
					self->hp = 0;
				self->direction = 0;
				return;
			}
			
			if (player_id < 0)
				return;

			int grav_x, grav_y;
			double grav_angle;
			
			//printf("%i %i\n", self->x, self->y);
			gcenter_calc(self->x/1000, self->y/1000, &grav_x, &grav_y);
			grav_angle = atan2(grav_y, grav_x);
			self->angle = -grav_angle * 1800 / M_PI + 900;

			bool left = 0, right = 0;

			left = ingame_keystate[player_id].left;
			right = ingame_keystate[player_id].right;

			if(s->player[player_id].fucked_controls) {
				bool temp;
				temp = right;
				right = left;
				left = temp;
			};

			if(s->player[player_id].stunned) {
				left = false;
				right = false;
			}

			//printf("player id %i is movable id %i\n", player_id, self->id);

			if (left) {
				double angle;

				angle = grav_angle + M_PI_2;
				
				self->x_velocity = 1000.0*cos(angle) + 40.0*cos(angle+M_PI_2);
				self->y_velocity = 1000.0*sin(angle) + 40.0*sin(angle+M_PI_2);
				//printf("walk %i %i %.4f\n", self->x_velocity, self->y_velocity, angle);

				//self->x_velocity = -300;// + block_property[s->player[player_id].holding->direction].mass/2;
				s->player[player_id].last_walk_direction = 0;
			} else if (right) {
				double angle;

				angle = grav_angle - M_PI_2;
				
				self->x_velocity = 1000.0*cos(angle) + 40.0*cos(angle - M_PI_2);
				self->y_velocity = 1000.0*sin(angle) + 40.0*sin(angle - M_PI_2);
				//printf("walk %i %i %.4f\n", self->x_velocity, self->y_velocity, angle);

				//self->x_velocity = 300;// - block_property[s->player[player_id].holding->direction].mass/2;
				s->player[player_id].last_walk_direction = 1;
			} else {
				self->x_velocity = 0;
				self->y_velocity = 0;
			}

			if (state->state == AI_PLAYER_WALKING) {
				if (ingame_keystate[player_id].jump) {
					DARNIT_KEYS keys;
				
					printf("Jump!\n");
					ingame_keystate[player_id].jump = 0;
					
					self->x_gravity = MOV_TERMINAL_VELOCITY * cos(grav_angle-M_PI);
					self->y_gravity = MOV_TERMINAL_VELOCITY * sin(grav_angle-M_PI);
					state->state = AI_PLAYER_LEAPING;
				}
			}

			if (state->state == AI_PLAYER_LEAPING) {
				if (self->gravity_blocked)
					state->state = AI_PLAYER_WALKING;
			}
			
			#if 0
			if (ingame_keystate[player_id].action && !self->y_velocity) {
				int x, y, area, tx, ty, tmx, tmy;
				area = self->x < 432000?0:1;
				tmx = (self->x - 24000)/1000%24;
				tmy = (self->y - 24000)/1000%24;
				tx = (self->x - 24000)/1000/24;
				ty = (self->y - 24000)/1000/24;
				if (tx>16)
					tx -= 18;
				if (!s->player[player_id].last_walk_direction)
					tx--;
				else if (tmx == 0)
					tx++;
				else
					tx += 2;
				if (!s->player[player_id].holding->direction) {
					fprintf(stderr, "Nothing to park here\n");
				} else if (blocklogic_find_place_site(area, tx, ty, s->player[player_id].last_walk_direction, 1, 1, &x, &y)) {
					s->block[area].block[x + BLOCKLOGIC_AREA_WIDTH * y] = s->player[player_id].holding->direction;
					block_place(x, y, area, s->player[player_id].holding->direction);

					s->player[player_id].holding->direction = 0;
				} else {
					fprintf(stderr, "can't park here\n");
				}
			}
			
			if (ingame_keystate[player_id].suicide) {
				_die(self, player_id);
			}
			
			s->player[player_id].holding->x = self->x;
			s->player[player_id].holding->y = self->y - 24000;
			#endif

			//noinput:
			
			if (movableTileCollision(self, 2, 0) & COLLISION_KILL ||
			    movableTileCollision(self, 0, -2) & COLLISION_KILL ||
			    movableTileCollision(self, -2, 0) & COLLISION_KILL ||
			    movableTileCollision(self, 0, 2) & COLLISION_KILL)
				_die(self, player_id);
				
			self->direction = _player_direction(self);
			break;
		case MOVABLE_MSG_CHECK_APPLES:
			if (state->apple[state->selected_apple] > 0) {
				state->apple[state->selected_apple]--;
				fm->direction = state->selected_apple + 1;
				_push_apple_count(state->apple, _get_player_id(self), state->selected_apple);
				s->movable.movable[s->player[_get_player_id(self)].bullet_movable].ai(fm, &s->movable.movable[s->player[_get_player_id(self)].bullet_movable], MOVABLE_MSG_FIRE);
			}
			break;
		case MOVABLE_MSG_CHANGE_APPLE:
			state->selected_apple++;
			if (state->selected_apple == 4)
				state->selected_apple = 0;
			_push_apple_count(state->apple, _get_player_id(self), state->selected_apple);
			printf("Changed to apple %i\n", state->selected_apple);
			
			break;
		case MOVABLE_MSG_DESTROY:
			/* TODO: Handle destroy, announce the dead. Etc. */
			break;
		default:
			break;

	}
}


static struct AILibEntry ailib[] = {
	{ "player", ai_player },
	{ "apple", ai_apple },
	{ "apple_bullet", ai_apple_bullet },
	{ NULL, NULL }
};

void *ailib_get(const char *str) {
	int i;

	for (i = 0; ailib[i].symbol; i++) {
		if (!strcmp(ailib[i].symbol, str))
			return (void *) ailib[i].func;
	}

	return NULL;
}


void ailib_torpedo(int movable) {
	int player_id;
	player_id = _get_player_id(&s->movable.movable[movable]);
	_die(&s->movable.movable[movable], player_id);
}

void ai_enemy(void *dummy, void *entry, MOVABLE_MSG msg) {
        MOVABLE_ENTRY *self = entry;
        int i;
        
        switch (msg) {
                case MOVABLE_MSG_INIT:
                        
                        break;
		case MOVABLE_MSG_LOOP:

			break;
                default:
                        break;
        }
}

