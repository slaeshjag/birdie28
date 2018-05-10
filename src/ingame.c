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
int ingame_timer_package_send(uint8_t advantage, uint32_t team1, uint32_t team2);
void ingame_timer_blit(int time_left, int mode, int pos);

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
	
	
	//d_render_offset(0, 0);
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
		if (newstate.jump)
			pressevent.jump = true, releaseevent.jump = false;
		else
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
			case PACKET_TYPE_EXIT:
				//game_over_set_team(pack.exit.team);
				game_state(GAME_STATE_GAME_OVER);
				break;
		}
	}
}


