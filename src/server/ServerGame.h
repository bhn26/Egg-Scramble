#pragma once
#include <stdlib.h>
#include "ServerNetwork.h"
#include "../network/NetworkData.h"
#include "engine/engine.h"
#include <btBulletDynamicsCommon.h>

#include <map>

class ServerGame
{

public:

    static unsigned int NumClients() {return client_id;}

    void update();

	void receiveFromClients();
    
    // Want singleton for the world, if we receive an init packet from a new client
    // we want to send them the current world data, not reset the world
    // Assume 1 client for now
    void receiveInitPacket(int offset);
    void sendInitPacket();

	void receiveJoinPacket(int offset);
	void sendJoinPacket(int client);

	// Starting the game packets
	void receiveStartPacket(int offset);
	void sendStartPacket();

    // The data we want in network_data should have an offset if any
    void sendSpawnPacket(PosInfo pi); // Spawn an object with position pi, pi holds obj type and obj id

	// Send what you want to remove, with the object's ids
	void sendRemovePacket(ClassId cid, int oid);

    // Returns the direction to be moved, if it can't move there, returns BAD_MOVE
    void receiveMovePacket(int offset);
	// what type is the object moving and what is the id of the object moving?
    void sendMovePacket(ClassId class_id, int obj_id);

    void receiveRotationPacket(int offset);
    void sendRotationPacket(int obj_id, float w, float x, float y, float z);

	void receiveJumpPacket(int offset);

	void sendScorePacket();

	static void instantiate()
	{
		if (sg == NULL)
			sg = new ServerGame();
	}

	static ServerGame* instance() { return sg; }

	void IncScore(int team, int n) { scores[team] += n; };
	void DecScore(int team, int n) { scores[team] -= n; ; }

	int GetTeam(int player) { return team_map[player]; };

private:
	ServerGame(void);
	~ServerGame(void);

    // IDs for the clients connecting for table in ServerNetwork 
    static unsigned int client_id;

	std::map <int, int> team_map; // <player, team>

	// Singleton servergame
	static ServerGame* sg;

	// variables for starting the game

	bool game_started = false;
	int ready_clients = 0; // # of clients ready for the game

	Engine * engine;

   // The ServerNetwork object 
    ServerNetwork* network;

	// data buffer
   char network_data[MAX_PACKET_SIZE];

   // SCORES
   int scores[2];
};