#pragma once

#include "EntitySpawner.h"
#include <vector>
#include <memory>
#include <map>

#include "../../network/GameData.h"
class WorldObstacle;

using namespace std;

typedef vector<PosInfo> pos_list;

class World {

private:

	// list of game world objects
	std::vector<std::shared_ptr<Flag>> flags;
	std::vector<Entity*> deleteList;  

	// Physics World attributes
	btDiscreteDynamicsWorld* curWorld;
	btDefaultCollisionConfiguration* colConfig;
	btCollisionDispatcher* disp;
	btBroadphaseInterface* pairCache;
	btSequentialImpulseConstraintSolver* solv;

	// Map objects
	WorldObstacle * ground;
	WorldObstacle * frontWall;
	WorldObstacle * backWall;
	WorldObstacle * leftWall;
	WorldObstacle * rightWall;

	// object ids
	int oid;

	// list of fields to check.  Explosions and mounts.  Maybe decouple into a class of it's own.  Have a TTL


public:
	World();
	~World();

	// ticker used for now
	int x = 0;
	int y = 0;

	void Init();

	void PreSpawn();
	btDiscreteDynamicsWorld* GetPhysicsWorld();

	// Updates Physics world by one tick
	void UpdateWorld();

	// Finds and Removes flag from world list of flags
	void removeFlag(Flag* collectedFlag);

};