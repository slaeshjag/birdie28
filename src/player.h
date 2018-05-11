#ifndef _PLAYER_H__
#define	_PLAYER_H__

#include <stdbool.h>
//#include "movable.h"

struct PlayerState {
	bool active;
	int team;
	int last_walk_direction;
	int pulling;
	int movable;

	int bullet_movable;

	int apple[4];
	int selected;

	bool fucked_controls;
	bool stunned;
	//MOVABLE_ENTRY *holding;
	//DARNIT_PARTICLE *blood;
};


#endif
