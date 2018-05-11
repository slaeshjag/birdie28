#include "main.h"
#include "ingame.h"
#include "camera.h"
#include "network/protocol.h"
#include <stdbool.h>
#include <string.h>
#include "network/network.h"
#include "network/protocol.h"
#include "server/server.h"
#include "main.h"
#include "gameover.h"
#include "util.h"
//#include "bullet.h"
//#include "turret.h"

static DARNIT_CIRCLE *_circle;

InGameKeyStateEntry ingame_keystate[PLAYER_CAP];
void ingame_timer_blit(int time_left, int mode, int pos);


void ingame_apple_bullet_fire() {
	int x, y;
	float angle;
	Packet pack;

	x = s->movable.movable[s->player[s->player_id].movable].x / 1000 + d_sprite_width(s->movable.movable[s->player[s->player_id].movable].sprite)/2;
	y = s->movable.movable[s->player[s->player_id].movable].y / 1000 + d_sprite_height(s->movable.movable[s->player[s->player_id].movable].sprite)/2;
	x -= s->camera.x;
	y -= s->camera.y;
	x -= d_mouse_get().x;
	y -= d_mouse_get().y;

	angle = atan2(y, x);
	x = -2000 * cos(angle);
	y = -2000 * sin(angle);

	pack.type = PACKET_TYPE_APPLE_BULLET_FIRE;
	pack.size = sizeof(PacketAppleBullet);
	pack.apple_bullet.xdir = x;
	pack.apple_bullet.ydir = y;
	protocol_send_packet(server_sock, (void *) &pack);

	//printf("Vector=%i %i\n", x, y);
}


void ingame_timer_package_send(int time_left) {
	Packet pack;
	pack.type = PACKET_TYPE_TIMER;
	pack.size = sizeof(PacketTimer);

	pack.timer.time_left = time_left;
	protocol_send_packet(server_sock, (void *) &pack);
}


void ingame_timer_blit(int time_left, int mode, int pos) {
	int minute, deka, second;

	minute = time_left / 60;
	deka = (time_left % 60) / 10;
	second = time_left % 10;
	d_render_tile_blit(s->_7seg, minute + 11*mode, pos, 0);
	d_render_tile_blit(s->_7seg, 10 + 11*mode, pos+24, 0);
	d_render_tile_blit(s->_7seg, deka + 11*mode, pos+48, 0);
	d_render_tile_blit(s->_7seg, second + 11*mode, pos+72, 0);
}

void ingame_applegague_blit(int player) {
	d_render_tile_blit(s->_applegague_bg, 0, 1280-260, 0);
}


void ingame_init() {
	int i;
	const char *playerid_str;
	/* Leak *all* the memory */
	s->active_level = d_map_load(util_binrel_path("res/map.ldmz"));
	s->camera.follow = -1;
	s->camera.x = s->camera.y = 0;

	movableInit();
//	bulletInit();
	movableLoad();
//	healthbar_init();
//	soundeffects_init();
//
	s->_7seg = d_render_tilesheet_load("res/7seg.png", 24, 32, DARNIT_PFORMAT_RGB5A1);
	s->_applegague_bg = d_render_tilesheet_load("res/applegauge.png", 260, 40, DARNIT_PFORMAT_RGB5A1);
	s->time_left = 1000 * 60 * 3;

	char *prop;
	int center_x, center_y, radius;
	prop = d_map_prop(s->active_level->prop, "center_x");
	center_x = atoi(prop);
	prop = d_map_prop(s->active_level->prop, "center_y");
	center_y = atoi(prop);
	prop = d_map_prop(s->active_level->prop, "radius");
	radius = atoi(prop);

	_circle = d_render_circle_new(1024, 10);
	printf("circle (%i, %i) radius %i\n", center_x, center_y, radius);
	d_render_circle_move(_circle, center_x, center_y, radius);

	for (i = 0; i < s->movable.movables; i++) {
		int id = _get_player_id(&s->movable.movable[i]);
		if (id == s->player_id) {
			printf("camera will follow movable %i\n");
			s->camera.follow = i;
			break;
		}
	}

}



void ingame_loop() {
	int i, team1t, team2t;
	
	d_render_clearcolor_set(0x88, 0xf2, 0xff);
	
	d_render_tint(255, 255, 255, 255);
	
	movableLoop();
	
	if(s->is_host) {
		server_kick();
		s->time_left -= d_last_frame_time();
		ingame_timer_package_send(s->time_left / 1000);
		
		if (s->time_left <= 0) {
			/* TODO: sätt gamestate till over */
		}
		//bullet_loop();
	//	turret_loop();
		
	}
	
	camera_work();
	d_map_camera_move(s->active_level, s->camera.x, s->camera.y);

	d_render_offset(s->camera.x, s->camera.y);
	d_render_circle_draw(_circle);
	
	for (i = 0; i < s->active_level->layers; i++) {
		d_render_offset(0, 0);
		d_render_tint(255, 255, 255, 255);
//		d_render_tile_blit(s->active_level->layer[i].ts, 0, 0, 1);
		d_tilemap_draw(s->active_level->layer[i].tilemap);
		d_render_offset(s->camera.x, s->camera.y);
		movableLoopRender(i);
		
	}
	

	d_render_offset(0, 0);
	ingame_timer_blit(s->time_left2 / 1000, 1, 0);
	ingame_applegague_blit(s->player_id);
//	healthbar_draw();
	ingame_client_keyboard();
}


void ingame_client_keyboard() {
	static struct InGameKeyStateEntry oldstate = {};
	struct InGameKeyStateEntry newstate, pressevent, releaseevent;

	memset(&pressevent, 0, sizeof(pressevent));
	memset(&releaseevent, 0, sizeof(pressevent));
	
	newstate.left = d_keys_get().left;
	newstate.right = d_keys_get().right;
	newstate.jump = d_keys_get().up;
	newstate.action = d_keys_get().a;
	newstate.suicide = d_keys_get().x;
	if (d_keys_get().lmb)
		ingame_apple_bullet_fire();
	
	if(d_keys_get().select)
		restart_to_menu(player_name);
	
	
	if (newstate.left ^ oldstate.left) {
		if (newstate.left)
			pressevent.left = true, releaseevent.left = false;
		else
			releaseevent.left = true, pressevent.left = false;
	}

	if (newstate.right ^ oldstate.right) {
		if (newstate.right)
			pressevent.right = true, releaseevent.right = false;
		else
			releaseevent.right = true, pressevent.right = false;
	}
	
	if (newstate.jump ^ oldstate.jump) {
		if (newstate.jump) {
			printf("newstate jump\n");
			pressevent.jump = true, releaseevent.jump = false;
		} else
			releaseevent.jump = true, pressevent.jump = false;
	}

	if (newstate.action ^ oldstate.action) {
		if (newstate.action)
			pressevent.action = true, releaseevent.action = false;
		else
			releaseevent.action = true, pressevent.action = false;
	}
	
	if (newstate.suicide ^ oldstate.suicide) {
		if (newstate.suicide)
			pressevent.suicide = true, releaseevent.suicide = false;
		else
			releaseevent.suicide = true, pressevent.suicide = false;
	}
	
	PacketKeypress kp;

	kp.size = sizeof(kp);
	kp.type = PACKET_TYPE_KEYPRESS;
	kp.keypress = pressevent;
	kp.keyrelease = releaseevent;

	protocol_send_packet(server_sock, (void *) &kp);

	oldstate = newstate;
}

void ingame_network_handler() {
	Packet pack;
	void *p;
	int i;
	
	while(network_poll_tcp(server_sock)) {
		
		protocol_recv_packet(server_sock, &pack);
		
		switch(pack.type) {
			case PACKET_TYPE_MOVE_OBJECT:
				p = pack.raw;
				
				for(i = 0; i < s->movable.movables; i++) {
					s->movable.movable[i].x = ((int) (*((uint16_t *) p))) * 1000;
					p+= 2;
					s->movable.movable[i].y = ((int) (*((uint16_t *) p))) * 1000;
					p+= 2;
					s->movable.movable[i].direction = *((uint8_t *) p);
					p+= 1;
					s->movable.movable[i].angle = *((uint8_t *) p);
					s->movable.movable[i].angle *= (2 * 10);
					p += 1;
					s->movable.movable[i].hp = *((uint16_t *) p);
					p += 2;
					s->movable.movable[i].hp_max = *((uint16_t *) p);
					p += 2;
				}
				break;
			
			case PACKET_TYPE_BULLET_ANNOUNCE:
//				if (s->is_host)
					//bullet_add(pack.bullet_announce.bullet_type, pack.bullet_announce.id, pack.bullet_announce.x, pack.bullet_announce.y);
				break;
			case PACKET_TYPE_BULLET_UPDATE:

//				if (s->is_host)
					//bullet_update(pack.bullet_update.x, pack.bullet_update.y, pack.bullet_update.id);
				break;
			case PACKET_TYPE_BULLET_REMOVE:
//				if (s->is_host)
					//bullet_destroy(pack.bullet_remove.id);
				break;
			case PACKET_TYPE_SOUND:
	//			soundeffects_play(pack.sound.sound);
				break;
			case PACKET_TYPE_TIMER:
				s->time_left2 = pack.timer.time_left * 1000;
				break;
			case PACKET_TYPE_EXIT:
				//game_over_set_team(pack.exit.team);
				game_state(GAME_STATE_GAME_OVER);
				break;
		}
	}
}


