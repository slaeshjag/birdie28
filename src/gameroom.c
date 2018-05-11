#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <darnit/darnit.h>
#include <muil/muil.h>
#include "network/network.h"
#include "network/protocol.h"
//#include "server/server.h"
#include "gameroom.h"
#include "main.h"

GameRoom gameroom;

static void button_callback(MuilWidget *widget, unsigned int type, MuilEvent *e) {
	if(widget == gameroom.button.back) {
		restart_to_menu(player_name);
	} else if(widget == gameroom.button.start) {
		server_start_game();
	}
}


void gameroom_init() {
	gameroom.pane.pane = muil_pane_create(10, 10, DISPLAY_WIDTH - 20, DISPLAY_HEIGHT - 20, gameroom.vbox = muil_widget_create_vbox());
	gameroom.pane.next = NULL;

	gameroom.pane.pane->background_color.r = PANE_R;
	gameroom.pane.pane->background_color.g = PANE_G;
	gameroom.pane.pane->background_color.b = PANE_B;

	muil_vbox_add_child(gameroom.vbox, gameroom.label = muil_widget_create_label(gfx.font.large, "Players in game"), 0);
	muil_vbox_add_child(gameroom.vbox, gameroom.list = muil_widget_create_listbox(gfx.font.small), 1);
	
	gameroom.hbox_button = muil_widget_create_hbox();
	
	muil_vbox_add_child(gameroom.hbox_button, gameroom.button.back = muil_widget_create_button_text(gfx.font.small, "Back"), 0);
	muil_vbox_add_child(gameroom.hbox_button, gameroom.button.start = muil_widget_create_button_text(gfx.font.small, "Start game"), 0);
	
	muil_vbox_add_child(gameroom.vbox, gameroom.hbox_button, 0);
	
	gameroom.button.start->enabled = false;
	
	gameroom.button.back->event_handler->add(gameroom.button.back, button_callback, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	gameroom.button.start->event_handler->add(gameroom.button.start, button_callback, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	
}


void gameroom_network_handler() {
	Packet pack;
	void *tmp;
	int i;
	
	if(!network_poll_tcp(server_sock))
		return;
	protocol_recv_packet(server_sock, &pack);
	
	switch(pack.type) {
		case PACKET_TYPE_JOIN:
			if(s->player[pack.join.id].active) {
				int i;
				MuilPropertyValue v;
				
				for(i = 0; i < v.i; i++) {
					if(atoi(muil_listbox_get(gameroom.list, i)) == pack.join.id) {
						asprintf(&tmp, "%i: %s", pack.join.id, pack.join.name);
						muil_listbox_set(gameroom.list, i, tmp);
						free(tmp);
						break;
					}
				}
				
			} else {
				asprintf(&tmp, "%i: %s", pack.join.id, pack.join.name);
				muil_listbox_add(gameroom.list, tmp);
				free(tmp);
				
				s->player[pack.join.id].active = true;
				strcpy(s->player[pack.join.id].name, pack.join.name);
			}
			break;
		
		case PACKET_TYPE_START:
			s->player_id = pack.start.player_id;
			printf("Started game as player %i\n", s->player_id);
			game_state(GAME_STATE_GAME);
			break;
	}
}
