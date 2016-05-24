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

	void receiveIndSpawnPacket(int offset);
	void sendReadyToSpawnPacket();

    // The data we want in network_data should have an offset if any
    void sendSpawnPacket(PosInfo pi); // Spawn an object with position pi, pi holds obj type and obj id

	// Send what you want to remove, with the object's ids
	void sendRemovePacket(ClassId cid, int oid);

    // Returns the direction to be moved, if it can't move there, returns BAD_MOVE
    void receiveMovePacket(int offset);
	// what type is the object moving and what is the id of the object moving?
    void sendMovePacket(ClassId class_id, int obj_id);

    void receiveRotationPacket(int offset);
    void sendRotationPacket(int class_id, int obj_id);

	void receiveJumpPacket(int offset);

	void receiveShootPacket(int offset);

	static void instantiate()
	{
		if (sg == NULL)
			sg = new ServerGame();
	}

	static ServerGame* instance() { return sg; }

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
	bool eggs_spawned = false;
	int ready_clients = 0; // # of clients ready for the game
	int spawned_clients = 0;

	Engine * engine;

   // The ServerNetwork object 
    ServerNetwork* network;

	// data buffer
   char network_data[MAX_PACKET_SIZE];

};