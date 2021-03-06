#pragma once

#include "BulletCollision\CollisionDispatch\btGhostObject.h"

#ifndef BULLET_PHYSICS
#define BULLET_PHYSICS
#include <BulletPhysics\btBulletDynamicsCommon.h>
#include <BulletPhysics\btBulletCollisionCommon.h>
#endif

class Entity;

class FieldObject
{
protected:
	btPairCachingGhostObject* FieldGhostObject;
	int team_id;
	btDiscreteDynamicsWorld* curWorld;

public:
	FieldObject(btVector3* origin, btCollisionShape* fieldshape, int team_id, btDiscreteDynamicsWorld* curworld);
	virtual ~FieldObject();

	btPairCachingGhostObject* GetFieldGhostObject() {return FieldGhostObject;}

	// handles field detection. Returns 1 when field is done, 0 if not
	virtual int handleField();
};


