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
	//MOVABLE_ENTRY *holding;
	//DARNIT_PARTICLE *blood;
};


#endif
