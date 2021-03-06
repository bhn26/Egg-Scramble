#pragma once
#include <string.h>
#include "GameData.h"

#define MAX_PACKET_SIZE 1000000
#define DATA_SIZE 100 // Change this to the largest data size that is needed

enum PacketTypes
{
    INIT_CONNECTION,  // Initial connection setup

	SET_USERNAME,

	JOIN_TEAM,

	START_GAME, // A client would send this to start the game

	SERVER_LOADING, // server sends to client

	READY_GAME, // The clients send this to say they're ready

	READY_TO_SPAWN_EVENT,

	IND_SPAWN_EVENT,

    SPAWN_EVENT,  // Creating a new object, sent to the client

	REMOVE_EVENT, // Removing an object, sent to the client

    MOVE_EVENT,

	TIME_EVENT,

    V_ROTATION_EVENT,

	JUMP_EVENT,

	ATTACK_EVENT,

	DISCARD_EVENT,

	DANCE_EVENT,

	DEATH_EVENT,

	RESPAWN_EVENT,

	UPDATE_SCORE,

	GAME_OVER
};

struct PacketHeader
{
    unsigned int packet_type;

    // id for server will be -1
    int sender_id;
    int receiver_id;

};

struct PacketData
{
	GameDataId game_data_id;
    char buf[DATA_SIZE];
};

struct Packet
{

    struct PacketHeader hdr;

    struct PacketData dat;


    void serialize(char * data) {
        memcpy(data, this, sizeof(Packet));
    }

    void deserialize(char * data) {
        memcpy(this, data, sizeof(Packet));
    }
};