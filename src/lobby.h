#ifndef LOBBY_H_
#define LOBBY_H_

#include <muil/muil.h>

typedef struct Lobby Lobby;
struct Lobby {
	struct MuilPaneList pane;
	MuilWidget *label;
	MuilWidget *list;
	MuilWidget *vbox;
	MuilWidget *hbox;
	struct {
		MuilWidget *join;
		MuilWidget *enter_ip;
		MuilWidget *back;
	} button;
};

typedef struct EnterIP EnterIP;
struct EnterIP {
	struct MuilPaneList pane;
	MuilWidget *label;
	MuilWidget *entry;
	MuilWidget *vbox;
	MuilWidget *hbox;
	struct {
		MuilWidget *join;
		MuilWidget *back;
	} button;
};

extern Lobby lobby;
extern EnterIP enter_ip;
void lobby_init();
void lobby_network_handler();

#endif
