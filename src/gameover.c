#include <stdbool.h>
#include <darnit/darnit.h>
#include <muil/muil.h>
#include "gameover.h"
#include "main.h"

GameOver game_over;

static void button_callback(MuilWidget *widget, unsigned int type, MuilEvent *e) {
	if(widget == game_over.button.menu) {
		restart_to_menu(player_name);
	}
}

void game_over_init() {
	game_over.pane.pane = muil_pane_create(10, 10, DISPLAY_WIDTH - 20, DISPLAY_HEIGHT - 20, game_over.vbox = muil_widget_create_vbox());
	game_over.pane.next = NULL;

	game_over.pane.pane->background_color.r = PANE_R;
	game_over.pane.pane->background_color.g = PANE_G;
	game_over.pane.pane->background_color.b = PANE_B;

	muil_vbox_add_child(game_over.vbox, game_over.label = muil_widget_create_label(gfx.font.large, "Game Over"), 0);
	muil_vbox_add_child(game_over.vbox, game_over.whowon = muil_widget_create_label(gfx.font.large, "Name won!"), 1);
	
	muil_vbox_add_child(game_over.vbox, game_over.button.menu = muil_widget_create_button_text(gfx.font.small, "Main menu"), 0);
	
	game_over.button.menu->event_handler->add(game_over.button.menu, button_callback, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
}

void game_over_set_team(int team) {
	MuilPropertyValue v;
	
	v.p = malloc(512);
	sprintf(v.p, "Team %s has won!", team_name[team]);
	
	game_over.whowon->set_prop(game_over.whowon, MUIL_LABEL_PROP_TEXT, v);
}
