#ifndef _AILIB_H_
#define	_AILIB_H_

#include "movable.h"

struct AILibEntry {
	const char		*symbol;
	void			(*func)(void *s, void *entry, MOVABLE_MSG msg);
};

void ai_player(void *dummy, void *entry, MOVABLE_MSG msg);

void *ailib_get(const char *str);
int _get_player_id(MOVABLE_ENTRY *self);
void ailib_torpedo(int movable);

#endif
