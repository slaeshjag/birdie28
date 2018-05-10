#ifndef GAMEOVER_H_
#define GAMEOVER_H_
#include <muil/muil.h>

typedef struct GameOver GameOver;
struct GameOver {
	struct MuilPaneList pane;
	MuilWidget *vbox;
	MuilWidget *label;
	MuilWidget *whowon;
	struct {
		MuilWidget *menu;
	} button;
};

extern GameOver game_over;
void game_over_init();
void game_over_set_team(int team);

#endif
