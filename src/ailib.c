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
#define	PLAYER_GRAB_RANGE 70

enum AI_APPLE_STATE {
	AI_APPLE_STATE_HIDING,
	AI_APPLE_STATE_RIPE,
	AI_APPLE_STATE_FALLING,
	AI_APPLE_STATE_PICKED,
	AI_APPLE_STATE_RESPAWN,
};


enum AI_APPLE_BULLET_STATE {
	AI_APPLE_BULLET_STATE_IDLE,
	AI_APPLE_BULLET_STATE_FLYING,
};


enum AI_PLAYER_STATE {
	AI_PLAYER_WALKING,
	AI_PLAYER_LEAPING,
	AI_PLAYER_PICKING_APPLE,
	AI_PLAYER_STUNNED,
	AI_PLAYER_FUCKED_CONTROLS,
};


struct AppleState {
	int			last_second;
	int			type;
	enum AI_APPLE_STATE 	state;
	int			picked_by;
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
	time_t			fucked_controls_time;
	int			apple_grabbed;
};

void ai_apple(void *dummy, void *entry, MOVABLE_MSG msg);

static void _player_grab_apple(MOVABLE_ENTRY *entry) {
	struct AIPlayerState *state = entry->mystery_pointer;
	int i, cx, cy, ax, ay, dx, dy, player;

	player = _get_player_id(entry);

	cx = s->movable.movable[s->player[player].movable].x / 1000;
	cy = s->movable.movable[s->player[player].movable].y / 1000;
	cx += util_sprite_xoff(s->movable.movable[s->player[player].movable].sprite);
	cy += util_sprite_yoff(s->movable.movable[s->player[player].movable].sprite);
	cx += util_sprite_width(s->movable.movable[s->player[player].movable].sprite)/2;
	cy += util_sprite_height(s->movable.movable[s->player[player].movable].sprite)/2;

	for (i = 0; i < s->movable.movables; i++) {
		if (s->movable.movable[i].ai != ai_apple)
			continue;
		ax = s->movable.movable[i].x / 1000 + d_sprite_width(s->movable.movable[i].sprite)/2;
		ay = s->movable.movable[i].y / 1000 + d_sprite_height(s->movable.movable[i].sprite)/2;
		dx = cx - ax;
		dy = cy - ay;
		if (dx * dx + dy * dy > PLAYER_GRAB_RANGE * PLAYER_GRAB_RANGE)
			continue;
		printf("found apple\n");
		s->movable.movable[i].ai((void *) _get_player_id(entry), &s->movable.movable[i], MOVABLE_MSG_APPLE_GRAB);
		if (state->state == AI_PLAYER_PICKING_APPLE)
			break;

	}
}

static void _trigger_effect(int x, int y, int player, int effect) {
	enum AI_PLAYER_STATE new_state = -1;
	
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
				//new_state = AI_PLAYER_STUNNED;

				s->movable.movable[s->player[i].movable].ai(NULL, &s->movable.movable[s->player[i].movable], MOVABLE_MSG_STUN);
				//state_struct->stunned_time = time(NULL);
				break;
			case EFFECT_DROP:
				s->movable.movable[s->player[i].movable].ai(NULL, &s->movable.movable[s->player[i].movable], MOVABLE_MSG_DROP);
				#if 0
				for(int j = 0; j < 4; j++) {
					s->player[i].apple[j] -= rand() % 2;
					if(s->player[i].apple[j] < 0)
						s->player[i].apple[j] = 0;
				}
				#endif
				break;
			case EFFECT_SLAPPED_AROUND:
				s->movable.movable[s->player[i].movable].ai(NULL, &s->movable.movable[s->player[i].movable], MOVABLE_MSG_SLAPPED_AROUND);
				//s->movable.movable[s->player[i].movable].x_velocity = (signed) ((rand() % 10000)) - 5000;
				//s->movable.movable[s->player[i].movable].y_velocity = (signed) ((rand() % 10000)) - 5000;
				break;
			case EFFECT_FUCKED_CONTROLS:
				s->movable.movable[s->player[i].movable].ai(NULL, &s->movable.movable[s->player[i].movable], MOVABLE_MSG_FUCKED_CONTROLS);
				//state_struct->fucked_controls_time = time(NULL);
				//new_state = AI_PLAYER_FUCKED_CONTROLS;
				break;
			default:
				printf("Unknown effect %i\n", effect);
		}
	}

	return new_state;
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
	return atoi(playerid_str);
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
			
			self->x = s->movable.movable[s->player[state->owner].movable].x;
			self->y = s->movable.movable[s->player[state->owner].movable].y;
			if (self->x < 0)
				self->x += (util_sprite_xoff(s->movable.movable[state->owner].sprite) + util_sprite_width(s->movable.movable[state->owner].sprite)/2 + d_sprite_width(self->sprite)/2)*1000;
				
			if (self->y < 0)
				self->y += (util_sprite_yoff(s->movable.movable[state->owner].sprite) + util_sprite_height(s->movable.movable[state->owner].sprite)/2 + d_sprite_height(self->sprite)/2)*1000;
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
			state = self->mystery_pointer;
			state->type = atoi(d_map_prop(s->active_level->object[self->id].ref, "type"));
			state->picked_by = -1;
			break;
		case MOVABLE_MSG_LOOP:
			if (self->flag) {
				self->flag = 0;
				return;
			}
		
			if (state->state == AI_APPLE_STATE_HIDING) {
				if (d_time_get()/1000 != state->last_second) {
					state->last_second = d_time_get()/1000;
					if (!(rand() % 1)) {
						state->state = AI_APPLE_STATE_RIPE;;
						self->direction = state->type + 1;
						self->gravity_effect = 0;
					}
				}
			} else if (state->state == AI_APPLE_STATE_RIPE) {
				if (d_time_get()/1000 != state->last_second) {
					state->last_second = d_time_get()/1000;
					if (!(rand() % 10)) {
						state->state = AI_APPLE_STATE_FALLING;
						printf("falling\n");
						state->last_second = d_time_get();
						self->gravity_effect = 1;
						self->tile_collision = 0;
					}
				}
			} else if (state->state == AI_APPLE_STATE_FALLING) {
				if (d_time_get() > state->last_second + 10000) {
					state->state = AI_APPLE_STATE_RESPAWN;
					printf("respawn\n");
				}
			} else if (state->state == AI_APPLE_STATE_RESPAWN) {
				s->movable.movable[self->id].x = s->active_level->object[self->id].x * 1000 * s->active_level->layer->tile_w;
				s->movable.movable[self->id].y = s->active_level->object[self->id].y * 1000 * s->active_level->layer->tile_h;
				self->y_velocity = self->x_velocity = self->x_gravity = self->y_gravity = 0;
				self->direction = 0;
				self->gravity_effect = 0;
				state->picked_by = -1;
				state->state = AI_APPLE_STATE_HIDING;
			} else if (state->state == AI_APPLE_STATE_PICKED) {
				if (d_time_get() > state->last_second + 1500) {
					state->state = AI_APPLE_STATE_RESPAWN;
					s->movable.movable[s->player[state->picked_by].movable].ai((void *) state->type, &s->movable.movable[s->player[state->picked_by].movable], MOVABLE_MSG_ADD_APPLE);
				}
			}

			//self->direction = ((d_time_get() / 2000) & 1);
			break;
		case MOVABLE_MSG_APPLE_GRAB:
			if (state->state != AI_APPLE_STATE_RIPE && state->state != AI_APPLE_STATE_FALLING)
				break;
			state->state = AI_APPLE_STATE_PICKED;
			state->picked_by = (int) dummy;
			state->last_second = d_time_get();
			s->movable.movable[s->player[(int) dummy].movable].ai((void *) self->id, &s->movable.movable[s->player[(int) dummy].movable], MOVABLE_MSG_GRABBED_APPLE);
			
			break;
		case MOVABLE_MSG_APPLE_ABORT:
			state->state = AI_APPLE_STATE_RESPAWN;
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

			if(state->state == AI_PLAYER_FUCKED_CONTROLS) {
				bool temp;
				temp = right;
				right = left;
				left = temp;
				if(state->fucked_controls_time > time(NULL))
					state->state = AI_PLAYER_WALKING;
			};
			
			if(state->state == AI_PLAYER_STUNNED) {
				if(state->stunned_time < d_time_get())
					state->state = AI_PLAYER_WALKING;
			}

			//printf("player id %i is movable id %i\n", player_id, self->id);
			if (state->state == AI_PLAYER_WALKING || state->state == AI_PLAYER_LEAPING || state->state == AI_PLAYER_FUCKED_CONTROLS) {
				if (ingame_keystate[player_id].action) {
					_player_grab_apple(entry);
					if (state->state == AI_PLAYER_PICKING_APPLE)
						break;
				}

				self->direction = _player_direction(self) | 2;
				if (left) {
					double angle;
	
					angle = grav_angle + M_PI_2;
					
					self->x_velocity = 1000.0*cos(angle) + 80.0*cos(angle+M_PI_2);
					self->y_velocity = 1000.0*sin(angle) + 80.0*sin(angle+M_PI_2);
					//printf("walk %i %i %.4f\n", self->x_velocity, self->y_velocity, angle);
	
					//self->x_velocity = -300;// + block_property[s->player[player_id].holding->direction].mass/2;
					s->player[player_id].last_walk_direction = 0;
				} else if (right) {
					double angle;

					angle = grav_angle - M_PI_2;
					
					self->x_velocity = 1000.0*cos(angle) + 80.0*cos(angle - M_PI_2);
					self->y_velocity = 1000.0*sin(angle) + 80.0*sin(angle - M_PI_2);
					//printf("walk %i %i %.4f\n", self->x_velocity, self->y_velocity, angle);

					//self->x_velocity = 300;// - block_property[s->player[player_id].holding->direction].mass/2;
					s->player[player_id].last_walk_direction = 1;
				} else {
					self->x_velocity = 0;
					self->y_velocity = 0;
					self->direction = _player_direction(self);
				}
			} else /* TODO: special case for "flying all over the place" */{
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
		
			if (state->state == AI_PLAYER_PICKING_APPLE) {
				self->gravity_effect = 0;
			}

			//noinput:
			
			if (movableTileCollision(self, 2, 0) & COLLISION_KILL ||
			    movableTileCollision(self, 0, -2) & COLLISION_KILL ||
			    movableTileCollision(self, -2, 0) & COLLISION_KILL ||
			    movableTileCollision(self, 0, 2) & COLLISION_KILL)
				_die(self, player_id);
				
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
		case MOVABLE_MSG_GRABBED_APPLE:
			state->apple_grabbed = (int) dummy;
			state->state = AI_PLAYER_PICKING_APPLE;
			break;
		case MOVABLE_MSG_ADD_APPLE:
			state->state = AI_PLAYER_WALKING;
			self->gravity_effect = 1;
			state->apple[(int) dummy]++;
			_push_apple_count(state->apple, _get_player_id(self), state->selected_apple);
			break;
		case MOVABLE_MSG_STUN:
			if (state->state == AI_PLAYER_PICKING_APPLE)
				s->movable.movable[state->apple_grabbed].ai(NULL, &s->movable.movable[state->apple_grabbed], MOVABLE_MSG_APPLE_ABORT);
			state->stunned_time = d_time_get() + 2000;
			state->state = AI_PLAYER_STUNNED;
			break;
		case MOVABLE_MSG_DROP:
			for(int j = 0; j < 4; j++) {
				state->apple[j] -= rand() % 2;
				if(state->apple[j] < 0)
					state->apple[j] = 0;
			}
			_push_apple_count(state->apple, _get_player_id(self), state->selected_apple);
			break;
		case MOVABLE_MSG_SLAPPED_AROUND:
			self->x_velocity = (signed) ((rand() % 10000)) - 5000;
			self->y_velocity = (signed) ((rand() % 10000)) - 5000;
			state->stunned_time = d_time_get() + 1000;
			state->state = AI_PLAYER_STUNNED;
			break;
		case MOVABLE_MSG_FUCKED_CONTROLS:
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

