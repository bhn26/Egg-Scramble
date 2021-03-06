#pragma once

#include <string.h>
#include <string>
#include <stdio.h>
#include "../server/engine/ObjectId.h"

const int WORLD_WIDTH = 230;
const int WORLD_HEIGHT = 500;

enum MoveType {

    BAD_MOVE = -1,

    MOVE_FORWARD = 0,

    MOVE_BACKWARD = 1,

    MOVE_LEFT = 2,

    MOVE_RIGHT = 3
};

static const int NUM_COLLECT_TYPES = 2; // Types of collectables there are

enum CollectType {
	WEAPONCOLLECT,
	POWERUPCOLLECT
};

static const int NUM_WEAPON_TYPES = 6; // number of types of weapons there are

enum WeaponType {
	SEEDGUN,
	BOUNCEGUN,
	GRENADELAUNCHER,
	TELEPORTGUN,
	BLASTMINE,
	SHOTGUN
};

static const int NUM_POWER_TYPES = 2;

enum PowerupType {
	HEALTHGAIN,
	JUMPUP
	//SPEEDUP
};

enum AttackType {
	PECK,

	WEAPON_ATTACK
};

enum GameDataId 
{
    POS_OBJ,
	REM_OBJ,
	SCORE_OBJ,
	EMOTE_OBJ,
	NAME_OBJ
};

struct GameInfo
{
    GameDataId id;

    // NETWORKING NOTE:
    // Usually, you want to serialize into PacketData's buf before you send
    void serialize(char * data) {
        memcpy(data, this, sizeof(GameInfo));
    }

    void deserialize(char * data) {
        memcpy(this, data, sizeof(GameInfo));
    }
};

// Position info of object
struct PosInfo : GameInfo
{
	// general info
	int id; // client id
	int team_id;

	int skin;

	int oid; // object id
	ClassId cid; // class id
	int sub_id; // this kind of id is used for subclasses, like different weapons spawning different bullets

	// object coordinates
    float x;
	float y;
	float z;

	int hp;

    int direction; // client -> server move data

	//rotation coords
	float rotw;
	float rotx;
	float roty;
	float rotz;

	float yos;

	int num_eggs; // num eggs this player has 
	int jump;     // jumping semaphore of the player

    void serialize(char * data) {
        memcpy(data, this, sizeof(PosInfo));
    }

    void deserialize(char * data) {
        memcpy(this, data, sizeof(PosInfo));
    }
};

// What needs to get removed
struct RemInfo : GameInfo
{
	int rem_oid;
	ClassId rem_cid;

	int rec_oid;    // What was responsible for the removal, i.e. player 2 collects an egg, p2's info goes here
	ClassId rec_cid;
	int team_id;    

	int sub_id;   // For removing collectable of type sub_id
	int sub_id2;  // For removing subtype of collectable, like subtypes of the weapon or powerup

	void serialize(char * data) {
		memcpy(data, this, sizeof(RemInfo));
	}

	void deserialize(char * data) {
		memcpy(this, data, sizeof(RemInfo));
	}
};

// team scores
struct ScoreInfo : GameInfo {
	int t0_score;
	int t1_score;

	void serialize(char * data) {
		memcpy(data, this, sizeof(ScoreInfo));
	}

	void deserialize(char * data) {
		memcpy(this, data, sizeof(ScoreInfo));
	}
};

// Who's emoting, also will include death
struct EmoteInfo : GameInfo {
	int id;

	void serialize(char * data) {
		memcpy(data, this, sizeof(EmoteInfo));
	}

	void deserialize(char * data) {
		memcpy(this, data, sizeof(EmoteInfo));
	}
};

// Anything that just needs some ints, i.e. attacktype
struct MiscInfo : GameInfo {
	int misc1;
	int misc2;
	float misc3;

	void serialize(char * data) {
		memcpy(data, this, sizeof(MiscInfo));
	}

	void deserialize(char * data) {
		memcpy(this, data, sizeof(MiscInfo));
	}
};

struct NameInfo : GameInfo {
	int player_id;
	std::string name;

	void serialize(char * data) {
		memcpy(data, this, sizeof(NameInfo));
	}

	void deserialize(char * data) {
		memcpy(this, data, sizeof(NameInfo));
	}
};