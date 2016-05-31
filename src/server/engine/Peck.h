#pragma once

#include "Weapon.h"
#include "EntitySpawner.h"

class Peck : public Weapon
{
protected:
	int meleefireRate = 50;
	int meleeDamage = 25;
public:
	Peck(btDiscreteDynamicsWorld* curWorld);
	~Peck();

	void virtual UseWeapon(btVector3* position, btMatrix3x3* rotation, int playerid, int teamid, Entity* owner);

	void ReloadWeapon();

};