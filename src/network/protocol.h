#ifndef __COMMON_PROTOCOL_H__
#define __COMMON_PROTOCOL_H__

#include <stdint.h>
#include "../ingame.h"
#include "../main.h"


typedef enum PacketType PacketType;
enum PacketType {
	PACKET_TYPE_LOBBY,
	PACKET_TYPE_JOIN,
	PACKET_TYPE_TEAM,
	PACKET_TYPE_START,
	PACKET_TYPE_MOVE_OBJECT,
	PACKET_TYPE_SOUND,
	PACKET_TYPE_KEYPRESS,
	PACKET_TYPE_BLOCK_PLACE,
	PACKET_TYPE_BLOOD,
	PACKET_TYPE_EXPLOSION,
	PACKET_TYPE_TIMER,
	PACKET_TYPE_APPLE_BULLET_FIRE,
	PACKET_TYPE_BULLET_ANNOUNCE,
	PACKET_TYPE_BULLET_UPDATE,
	PACKET_TYPE_BULLET_REMOVE,
	PACKET_TYPE_EXIT,
};

typedef struct PacketLobby PacketLobby;
struct PacketLobby {
	uint16_t type;
	uint16_t size;
	
	char name[NAME_LEN_MAX];
};

typedef struct PacketJoin PacketJoin;
struct PacketJoin {
	uint16_t type;
	uint16_t size;
	
	uint32_t id;
	char name[NAME_LEN_MAX];
	int team;
};

typedef struct PacketStart PacketStart;
struct PacketStart {
	uint16_t type;
	uint16_t size;
	
	uint32_t player_id;
};

typedef struct PacketKeypress PacketKeypress;
struct PacketKeypress {
	uint16_t type;
	uint16_t size;

	InGameKeyStateEntry keypress, keyrelease;
};

typedef struct PacketBlood PacketBlood;
struct PacketBlood {
	uint16_t type;
	uint16_t size;
	
	uint32_t player;
	uint32_t x;
	uint32_t y;
};

typedef struct PacketExplosion PacketExplosion;
struct PacketExplosion {
	uint16_t type;
	uint16_t size;
	
	uint32_t team;
	uint32_t x;
	uint32_t y;
};

typedef struct PacketSound PacketSound;
struct PacketSound {
	uint16_t type;
	uint16_t size;

	uint8_t sound;
};


typedef struct PacketBlockPlace PacketBlockPlace;
struct PacketBlockPlace {
	uint16_t type;
	uint16_t size;
	
	uint8_t team;
	uint8_t	x;
	uint8_t	y;
	uint8_t block;
};


typedef struct PacketBulletAnnounce PacketBulletAnnounce;
struct PacketBulletAnnounce {
	uint16_t type;
	uint16_t size;

	uint8_t bullet_type;
	uint16_t x;
	uint16_t y;
	uint8_t id;
};


typedef struct PacketBulletRemove PacketBulletRemove;
struct PacketBulletRemove {
	uint16_t type;
	uint16_t size;

	uint8_t id;
};


typedef struct PacketBulletUpdate PacketBulletUpdate;
struct PacketBulletUpdate {
	uint16_t type;
	uint16_t size;

	uint16_t x;
	uint16_t y;
	uint8_t id;
};


typedef struct PacketTimer PacketTimer;
struct PacketTimer {
	uint16_t type;
	uint16_t size;

	uint32_t time_left;
};


typedef struct PacketAppleBullet PacketAppleBullet;
struct PacketAppleBullet {
	uint16_t type;
	uint16_t size;

	int32_t xdir;
	int32_t ydir;
};


typedef struct PacketExit PacketExit;
struct PacketExit {
	uint16_t type;
	uint16_t size;
	
	uint32_t team;
};

typedef union Packet Packet;
union Packet {
	struct {
		uint16_t type;
		uint16_t size;
		uint8_t raw[1024];
	};
	
	PacketLobby lobby;
	PacketJoin join;
	PacketStart start;
	PacketSound sound;
	PacketKeypress keypress;
	PacketExit exit;
	PacketBlockPlace block_place;
	PacketExplosion explosion;
	PacketBlood blood;
	PacketTimer timer;
	PacketBulletUpdate bullet_update;
	PacketBulletAnnounce bullet_announce;
	PacketBulletRemove bullet_remove;
	PacketAppleBullet apple_bullet;
};

int protocol_send_packet(int sock, Packet *pack);
int protocol_recv_packet(int sock, Packet *pack);

#endif
