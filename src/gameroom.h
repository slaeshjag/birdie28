#ifndef GAMEROOM_H_
#define GAMEROOM_H_
#include <muil/muil.h>

typedef struct GameRoom GameRoom;
struct GameRoom {
	struct MuilPaneList pane;
	MuilWidget *label;
	MuilWidget *list;
	MuilWidget *vbox;
	MuilWidget *hbox_team;
	MuilWidget *hbox_button;
	struct {
		MuilWidget *start;
		MuilWidget *back;
	} button;
	
	struct {
		MuilWidget *list;
		MuilWidget *label;
	} team;
	
	DARNIT_SPRITE *sprite[4];
};

extern GameRoom gameroom;
void gameroom_network_handler();
void gameroom_init();

#endif
