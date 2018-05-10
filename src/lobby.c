#include <string.h>
#include <darnit/darnit.h>
#include <muil/muil.h>
#include "network/protocol.h"
#include "network/network.h"
#include "main.h"
#include "lobby.h"

Lobby lobby;
EnterIP enter_ip;

static void button_callback_lobby(MuilWidget *widget, unsigned int type, MuilEvent *e) {
	MuilPropertyValue v;
	
	if(widget == lobby.button.back) {
		restart_to_menu(player_name);
	} else if(widget == lobby.button.enter_ip) {
		game_state(GAME_STATE_ENTER_IP);
	} else if(widget == lobby.button.join) {
		v = lobby.list->get_prop(lobby.list, MUIL_LISTBOX_PROP_SELECTED);
		if(v.i < 0)
			return;
		
		unsigned long sip = strtoul(muil_listbox_get(lobby.list, v.i), NULL, 10);
		
		if(join_game(sip) >= 0)
			game_state(GAME_STATE_GAMEROOM);
	}
}

static void button_callback_enter_ip(MuilWidget *widget, unsigned int type, MuilEvent *e) {
	MuilPropertyValue v;
	
	if(widget == enter_ip.button.back) {
		restart_to_menu(player_name);
	} else if(widget == enter_ip.button.join) {
		v = enter_ip.entry->get_prop(enter_ip.entry, MUIL_ENTRY_PROP_TEXT);
		if(!v.p)
			return;
		
		unsigned long sip = network_ip(v.p);
		
		if(join_game(sip) >= 0)
			game_state(GAME_STATE_GAMEROOM);
	}
}

void lobby_init() {
	lobby.pane.pane = muil_pane_create(10, 10, DISPLAY_WIDTH - 20, DISPLAY_HEIGHT - 20, lobby.vbox = muil_widget_create_vbox());
	lobby.pane.next = NULL;
	
	lobby.pane.pane->background_color.r = PANE_R;
	lobby.pane.pane->background_color.g = PANE_G;
	lobby.pane.pane->background_color.b = PANE_B;
	
	muil_vbox_add_child(lobby.vbox, lobby.label = muil_widget_create_label(gfx.font.large, "Join game"), 0);
	muil_vbox_add_child(lobby.vbox, lobby.list = muil_widget_create_listbox(gfx.font.small), 1);
	
	lobby.hbox = muil_widget_create_hbox();
	muil_vbox_add_child(lobby.hbox, lobby.button.back = muil_widget_create_button_text(gfx.font.small, "Back"), 0);
	muil_vbox_add_child(lobby.hbox, lobby.button.join = muil_widget_create_button_text(gfx.font.small, "Join"), 0);
	muil_vbox_add_child(lobby.hbox, muil_widget_create_spacer(), 1);
	muil_vbox_add_child(lobby.hbox, lobby.button.enter_ip = muil_widget_create_button_text(gfx.font.small, "Enter IP"), 0);
	muil_vbox_add_child(lobby.vbox, lobby.hbox, 0);
	
	lobby.button.back->event_handler->add(lobby.button.back, button_callback_lobby, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	lobby.button.join->event_handler->add(lobby.button.join, button_callback_lobby, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	lobby.button.enter_ip->event_handler->add(lobby.button.enter_ip, button_callback_lobby, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	
	enter_ip.pane.pane = muil_pane_create(10, 40, DISPLAY_WIDTH - 20, 200, enter_ip.vbox = muil_widget_create_vbox());
	enter_ip.pane.next = NULL;
	
	enter_ip.pane.pane->background_color.r = PANE_R;
	enter_ip.pane.pane->background_color.g = PANE_G;
	enter_ip.pane.pane->background_color.b = PANE_B;
	
	muil_vbox_add_child(enter_ip.vbox, enter_ip.label = muil_widget_create_label(gfx.font.small, "Enter IP address"), 1);
	muil_vbox_add_child(enter_ip.vbox, enter_ip.entry = muil_widget_create_entry(gfx.font.small), 0);
	
	enter_ip.hbox = muil_widget_create_hbox();
	muil_hbox_add_child(enter_ip.hbox, enter_ip.button.back = muil_widget_create_button_text(gfx.font.small, "Back"), 0);
	muil_hbox_add_child(enter_ip.hbox, enter_ip.button.join = muil_widget_create_button_text(gfx.font.small, "Join"), 0);
	muil_vbox_add_child(enter_ip.vbox, enter_ip.hbox, 0);
	
	enter_ip.button.back->event_handler->add(enter_ip.button.back, button_callback_enter_ip, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	enter_ip.button.join->event_handler->add(enter_ip.button.join, button_callback_enter_ip, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
}

void lobby_network_handler() {
	MuilPropertyValue v;
	Packet pack;
	char name[256], *s;
	unsigned long ip;
	int i;
	
	if(!network_poll_udp())
		return;
	ip = network_recv_udp(&pack, sizeof(PacketLobby));
	if(pack.type != PACKET_TYPE_LOBBY)
		return;
	
	v = lobby.list->get_prop(lobby.list, MUIL_LISTBOX_PROP_SIZE);
	for(i = 0; i < v.i; i++) {
		s = muil_listbox_get(lobby.list, i);
		if(strtoul(s, NULL, 10) == ip) {
			if(strstr(s, "Unknown")) {
				sprintf(name, "%lu: %s's game", ip, pack.lobby.name);
				muil_listbox_set(lobby.list, i, name);
				return;
			} else
				return;
		}
	}
	sprintf(name, "%lu: %s's game", ip, pack.lobby.name);
	muil_listbox_add(lobby.list, name);
}
