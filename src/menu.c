#include <darnit/darnit.h>
#include <muil/muil.h>
#include "main.h"
#include "menu.h"

Menu menu;
SelectName select_name;
MenuHelp menu_help;

static void button_callback_name(MuilWidget *widget, unsigned int type, MuilEvent *e) {
	MuilPropertyValue v;
	if(widget == select_name.button.quit) {
		d_quit();
	}
	v = select_name.entry->get_prop(select_name.entry, MUIL_ENTRY_PROP_TEXT);
	snprintf(player_name, NAME_LEN_MAX, "%s", (char *) v.p);
	game_state(GAME_STATE_MENU);
}

static void button_callback_menu(MuilWidget *widget, unsigned int type, MuilEvent *e) {
	/*MuilPropertyValue v;
	v.p = "arne";
	label->set_prop(label, MUIL_LABEL_PROP_TEXT, v);*/
	
	if(widget == menu.button.host) {
		game_state(GAME_STATE_HOST);
	} else if(widget == menu.button.join) {
		game_state(GAME_STATE_LOBBY);
	} else if(widget == menu.button.character) {
		game_state(GAME_STATE_CHARACTERS);
	} else if(widget == menu.button.quit)
		d_quit();
}

void menu_init() {
	MuilPropertyValue v;
	menu.pane.pane = muil_pane_create(10, 10, DISPLAY_WIDTH - 20, 220, menu.vbox = muil_widget_create_vbox());
	menu.pane.next = &menu_help.pane;
	
	menu.pane.pane->background_color.r = PANE_R;
	menu.pane.pane->background_color.g = PANE_G;
	menu.pane.pane->background_color.b = PANE_B;
	
	menu_help.pane.pane = muil_pane_create(10, 210 + 30, DISPLAY_WIDTH - 20, DISPLAY_HEIGHT - 210 - 20 - 20, menu_help.vbox = muil_widget_create_vbox());
	menu_help.pane.next = NULL;
	
	menu_help.pane.pane->background_color.r = PANE_R;
	menu_help.pane.pane->background_color.g = PANE_G;
	menu_help.pane.pane->background_color.b = PANE_B;
	
	muil_vbox_add_child(menu_help.vbox, menu_help.label.title = muil_widget_create_label(gfx.font.large, "How to play"), 0);
	muil_vbox_add_child(menu_help.vbox, muil_widget_create_spacer_size(0, 10), 0);
	muil_vbox_add_child(menu_help.vbox, menu_help.label.title = muil_widget_create_label(gfx.font.small, 
		"nope.avi\n"
	), 0);
	muil_vbox_add_child(menu_help.vbox, muil_widget_create_spacer(), 1);
	muil_vbox_add_child(menu_help.vbox, menu_help.label.title = muil_widget_create_label(gfx.font.small, "2018 party hack for Birdie Gamedev compo\nh4xxel (http://h4xxel.org) & slaesjag (http://rdw.se) of //actung fulkod"), 0);
	
	muil_vbox_add_child(menu.vbox, menu.label.title = muil_widget_create_label(gfx.font.large, GAME_TITLE), 1);
	muil_vbox_add_child(menu.vbox, menu.label.author = muil_widget_create_label(gfx.font.small, "//actung fulkod"), 1);
	//muil_vbox_add_child(menu.vbox, muil_widget_create_entry(font), 0);
	muil_vbox_add_child(menu.vbox, menu.button.host = muil_widget_create_button_text(gfx.font.small, "Host game"), 0);
	muil_vbox_add_child(menu.vbox, menu.button.join = muil_widget_create_button_text(gfx.font.small, "Join game"), 0);
	//muil_vbox_add_child(menu.vbox, menu.button.character = muil_widget_create_button_text(gfx.font.small, "Character Showcase"), 0);
	muil_vbox_add_child(menu.vbox, menu.button.quit = muil_widget_create_button_text(gfx.font.small, "Quit game"), 0);

	menu.button.host->event_handler->add(menu.button.host, button_callback_menu, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	menu.button.join->event_handler->add(menu.button.join, button_callback_menu, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	//menu.button.character->event_handler->add(menu.button.character, button_callback_menu, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	menu.button.quit->event_handler->add(menu.button.quit, button_callback_menu, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
        
        
	select_name.pane.pane = muil_pane_create(DISPLAY_WIDTH/2 - 200, DISPLAY_HEIGHT/2 - 150/2, 400, 150, select_name.vbox = muil_widget_create_vbox());
	select_name.pane.next = NULL;
	
	select_name.pane.pane->background_color.r = PANE_R;
	select_name.pane.pane->background_color.g = PANE_G;
	select_name.pane.pane->background_color.b = PANE_B;
	
	muil_vbox_add_child(select_name.vbox, select_name.label = muil_widget_create_label(gfx.font.large, "Enter a name"), 1);
	muil_vbox_add_child(select_name.vbox, select_name.entry = muil_widget_create_entry(gfx.font.small), 0);
	select_name.hbox = muil_widget_create_hbox();
	muil_vbox_add_child(select_name.hbox, select_name.button.ok = muil_widget_create_button_text(gfx.font.small, "OK"), 0);
	muil_vbox_add_child(select_name.hbox, select_name.button.quit = muil_widget_create_button_text(gfx.font.small, "Quit"), 0);
	muil_vbox_add_child(select_name.vbox, select_name.hbox, 0);
	
	#ifdef _WIN32
	v.p = getenv("USERNAME");
	#else
	v.p = getenv("LOGNAME");
	#endif
	select_name.entry->set_prop(select_name.entry, MUIL_ENTRY_PROP_TEXT, v);
	
	select_name.button.ok->event_handler->add(select_name.button.ok, button_callback_name, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
	select_name.button.quit->event_handler->add(select_name.button.quit, button_callback_name, MUIL_EVENT_TYPE_UI_WIDGET_ACTIVATE);
}

void menu_render() {
	
}
