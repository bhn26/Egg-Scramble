#pragma once

#include "Entity.h"
#include "../../network/GameData.h"


class Bullet : public Entity
{
private:
	int playerId;
	int teamId;
	int damage;

public:

	Bullet(int objectid, int playerid, int teamid, int damage, btVector3* pos, btVector3* velocity, btMatrix3x3* rotation, btDiscreteDynamicsWorld* physicsWorld);

	virtual ~Bullet();

	btVector3 GetBulletPosition();

	//maybe handleBulletCollision(obA, obB);

	// get player id
	int GetPlayerId();

	// get team id
	int GetTeamId();

	int GetDamage();
};