#ifndef __MOVABLE_H__
#define	__MOVABLE_H__

#include <darnit/darnit.h>

#define GRAVITY_CAP		40
#define	GRAVITY_SCALE		1

#define	COLLISION_NONE	0x00000
#define	COLLISION_BOTTOM	0x10000
#define	COLLISION_LEFT		0x20000
#define	COLLISION_TOP		0x40000
#define	COLLISION_RIGHT		0x80000
#define	COLLISION_KILL		0x100000
#define	COLLISION_CHECKPOINT	0x200000

#define COLLISION_ALL (COLLISION_LEFT | COLLISION_RIGHT | COLLISION_TOP | COLLISION_BOTTOM)

#define	MOV_TERMINAL_VELOCITY	40000
#define	MOV_GRAVITY_STRONG	2000
#define	MOV_GRAVITY_WEAK	1000


typedef enum {
	MOVABLE_MSG_INIT,
	MOVABLE_MSG_LOOP,
	MOVABLE_MSG_COLLISION,
	MOVABLE_MSG_FIRE,
	MOVABLE_MSG_CHECK_APPLES,
	MOVABLE_MSG_REQUEST_FIRE,
	MOVABLE_MSG_CHANGE_APPLE,
	MOVABLE_MSG_APPLE_GRAB,
	MOVABLE_MSG_GRABBED_APPLE,
	MOVABLE_MSG_APPLE_ABORT,
	MOVABLE_MSG_ADD_APPLE,
	MOVABLE_MSG_STUN,
	MOVABLE_MSG_DROP,
	MOVABLE_MSG_SLAPPED_AROUND,
	MOVABLE_MSG_FUCKED_CONTROLS,
	MOVABLE_MSG_DESTROY
} MOVABLE_MSG;


typedef struct {
	int			x;
	int			y;
	int			w;
	int			h;
} MOVABLE_HITBOX;


typedef struct {
	int			id;
	int			bbox;
	DARNIT_SPRITE		*sprite;
	int			x;
	int			y;
	int			l;
	int			w;
	int			h;
	int			x_off;
	int			y_off;
	int			direction;
	int			x_velocity;
	int			y_velocity;
	int			x_gravity;
	int			y_gravity;
	int			gravity_effect;
	int			tile_collision;
	int			hp;
	int			hp_max;
	int			hit;
	int			hit_time;
	int			type;
	int			touch_dmg;
	int			prevent_respawn;
	int			angle;
	void			(*ai)(void *s, void *entry, MOVABLE_MSG msg);
	void			*mystery_pointer;
	int			flag;
	int			gravity_blocked;
	int			movement_blocked;
} MOVABLE_ENTRY;
	

typedef struct {
	MOVABLE_ENTRY		*movable;
	unsigned int		*coll_buf;
	unsigned int		*ai_coll_buf;
	DARNIT_BBOX		*bbox;
	int			movables;
	int			respawn_x;
	int			respawn_y;
} MOVABLE;

int movableInit();
int movableLoad();
void movableSpawn();
void movableKillEmAll();
int movableGravity(MOVABLE_ENTRY *entry);
void movableLoop();
void movableLoopRender(int layer);
int movableTileCollision(MOVABLE_ENTRY *entry, int off_x, int off_y);
void movableFreezeSprites(int freeze);
void movableRespawn();


#endif
