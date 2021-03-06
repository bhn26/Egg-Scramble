#include "EntitySpawner.h"
#include "ObjectId.h"

#include "server\ServerGame.h"

#include "Player.h"
#include "Flag.h"
#include "Peck.h"
#include "SeedGun.h"
#include "GrenadeLauncher.h"
#include "RespawnHandler.h"
#include <time.h>
#include "../ServerGame.h"


Player::Player(int objectid, int teamid, PosInfo pos, btDiscreteDynamicsWorld* physicsWorld) : Entity(ClassId::PLAYER, objectid, physicsWorld) 
{
	btCollisionShape* playerShape = new btSphereShape(1);//btCylinderShape(btVector3(1, 1, 1));
	// Create player physics object
	btDefaultMotionState*playerMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(pos.x, 90, pos.z)));
	btScalar mass = 100;
	btVector3 playerInertia(0, 0, 0);
	playerShape->calculateLocalInertia(mass, playerInertia);
	btRigidBody::btRigidBodyConstructionInfo playerRigidBodyCI(mass, playerMotionState, playerShape, playerInertia);
	btRigidBody* pRigidBody = new btRigidBody(playerRigidBodyCI);
	// only allow rotation around Y-axis
	pRigidBody->forceActivationState(DISABLE_DEACTIVATION);
    pRigidBody->setDamping((btScalar)0.8, (btScalar)60.0);
	pRigidBody->setFriction((btScalar) 0);
	physicsWorld->addRigidBody(pRigidBody);

	btTransform currentTrans;
	playerMotionState->getWorldTransform(currentTrans);
	btQuaternion currentOrientation = currentTrans.getRotation();
	this->cameraAngle = currentOrientation;

	// Set Player protected fields
	this->entityRigidBody = pRigidBody;
	this->teamId = teamid;
	this->jumpSem = 2;
	this->hitPoints = 100;
	this->flags = new std::vector<Flag*>;
	this->position = pos;
	this->playerWeapon = nullptr;
	this->peckWeapon = new Peck(curWorld);
	this->alive = true;
	this->death_time = 0;
	this->stun_count = 0;

	// Set Base and Bonus speed and jump
	this->baseJump = 40;
	this->baseSpeed = 25;
	this->speedPenalty = 0;
	this->lowestSpeed = 12;
	this->bonusJump = 0;
	this->bonusSpeed = 0;

	// Set RigidBody to point to Player
	pRigidBody->setUserPointer(this);
	pRigidBody->setUserIndex(PLAYER);
}

Player::~Player() {
	this->curWorld->removeCollisionObject(entityRigidBody);
	delete entityRigidBody->getMotionState();
	delete entityRigidBody->getCollisionShape();
	delete entityRigidBody;
	flags->clear();
	delete flags;
}

void Player::PrintPlayerVelocity()
{
	//Calculate new velocity
	btTransform currentTrans;
	entityRigidBody->getMotionState()->getWorldTransform(currentTrans);
	btMatrix3x3 currentOrientation = currentTrans.getBasis();
	//printf("current velocity %f, %f, %f\n", float(entityRigidBody->getLinearVelocity()[0]), float(playerRigidBody->getLinearVelocity()[1]), float(playerRigidBody->getLinearVelocity()[2]));
}

void Player::JumpPlayer()
{
	if (!alive)
		return;

	if (jumpSem)
	{
		// Change jump semaphore, change upward y-axis velocity
		jumpSem--;
		btVector3 curVelocity = entityRigidBody->getLinearVelocity();
		// setting upward velocity to 3
		curVelocity[1] = this->GetPlayerJump();
		entityRigidBody->setLinearVelocity(curVelocity);
	}
}

void Player::ResetJump()
{
	(this->jumpSem) = 2;
}

void Player::AcquireFlag(Flag* flag)
{
	if (!alive)
		return;
	// player collects flag, remove from entity list
	flags->push_back(flag);
	switch (flags->size()) {
		case 1:
			speedPenalty = 1;
			break;
		case 2:
			speedPenalty = 3;
			break;
		case 3:
			speedPenalty = 6;
			break;
		case 4:
			speedPenalty = 10;
			break;
		case 5:
			speedPenalty = 15;
			break;
		default:
			speedPenalty = 15;
			break;
	}
	printf("FLAG ACQUIRED\n");

	// note - individual scores are updated with move packets
	ServerGame::instance()->IncScore(teamId, 1);
	//ServerGame::instance()->sendScorePacket();

	// check if your team won
	int * scores = ServerGame::instance()->GetScores();
	printf("Team %d has %d out of %d eggs\n", teamId, scores[teamId], ServerGame::instance()->GetTotalEggs());
	if(scores[teamId] == ServerGame::instance()->GetTotalEggs()) {
		printf("sending game over packet\n");
		ServerGame::instance()->sendGameOverPacket(teamId);
	}
}

void Player::LoseFlags()
{
	ServerGame::instance()->DecScore(teamId, flags->size());

	// Change this, we need the flags to come out of the player back into the world
	flags->clear();
	//ServerGame::instance()->sendScorePacket();
}

int Player::GetTeamId()
{
	return teamId;
}

 void Player::UseWeapon()
{
	if (!alive)
		return;

	if(stun_count > 0)
		return;

	//printf("Player %u : attempting to use weapon\n", objectId);
	// If player weapon doesn't exist, exit
	if(!playerWeapon)
	{
		return;
	}

	// passes player position when using weapon
	btVector3 temp = this->GetEntityPosition();

	btQuaternion * playerRotation = &(this->cameraAngle);

	btVector3* position = new btVector3(temp.getX(), temp.getY(), temp.getZ());

	btMatrix3x3 * currentOrientation = new btMatrix3x3(*playerRotation);

	//ServerGame::instance()->sendShootPacket(objectId);
	if (playerWeapon->UseWeapon(position, currentOrientation, this->objectId, this->teamId, this) == 0)
	{
		ServerGame::instance()->sendDiscardPacket(this->GetObjectId());
		delete playerWeapon;
		playerWeapon = nullptr;
	}
	ServerGame::instance()->sendAttackPacket(objectId);
}

 void Player::SetCamAngle(float yos)
 {
	 btQuaternion playerRotation = this->GetEntityRotation();
	 if (yos > -9990)
	 {
		 yos += .2;
		 printf("Camera Vertical Angle is: %f\n", yos);
		 this->cameraAngle = (playerRotation) * (btQuaternion(btVector3(-1, 0, 0), yos));
	 }
 }
 
void Player::EquipWeapon(Weapon* newWeapon)
{
	if (!alive)
		return;
	this->playerWeapon = newWeapon;
}

void Player::EquipPower(Powerup* powerup)
{
	if (!alive)
		return;
	this->power = powerup;
}

void Player::LosePower()
{
	if (power)
	{
		delete power;
		power = nullptr;
	}
}

void Player::DiscardWeapon()
{
	if (playerWeapon)
	{
		delete playerWeapon;
		playerWeapon = nullptr;
		ServerGame::instance()->sendDiscardPacket(objectId);
	}

	//if(alive) send packet for discard animation
}

bool Player::HasWeapon()
{
	return (this->playerWeapon != nullptr);
}

bool Player::HasPower()
{
	return (this->power != nullptr);
}

 // If player is dead, returns 1,  else returns 0
int Player::takeDamage(int damage, unsigned int world_tick)
{
	if (!alive)
		return 0;

	this->hitPoints = this->hitPoints - damage;

	if (this->hitPoints <= 0)
	{
		this->HandleDeath(world_tick);
		return 1;
	}
	else
	{
		return 0;
	}
}

void Player::UsePeck()
{

	if (!alive)
		return;

	if (stun_count > 0)
		return;

	btVector3 temp = this->GetEntityPosition();

	btQuaternion * playerRotation = &(this->GetEntityRotation());
	//playerRotation->setX(position.camx);
	//playerRotation->setZ(position.camz);

	btVector3* position = new btVector3(temp.getX(), temp.getY(), temp.getZ());

	btTransform currentTrans;
	entityRigidBody->getMotionState()->getWorldTransform(currentTrans);
	btMatrix3x3 * currentOrientation = new btMatrix3x3(*playerRotation);

	peckWeapon->UseWeapon(position, currentOrientation, this->objectId, this->teamId, this);
	ServerGame::instance()->sendAttackPacket(objectId);
	//	peckWeapon->UseWeapon(&(entityRigidBody->getCenterOfMassPosition()), &currentOrientation, this->objectId, this->teamId, this);
	//printf("player with objId: %d used peck! \n", objectId);
}

void Player::HandleDeath(unsigned int death_tick)
{
	//printf("Player %u has died!", objectId);
	this->alive = false;
	speedPenalty = 0;
	DiscardWeapon();
	LosePower();
	death_time = death_tick;
	ServerGame::instance()->sendDeathPacket(objectId);
	//EntitySpawner::instance()->RemoveEntity(classId, objectId);

	RespawnHandler::instance()->KillPlayer(this);   // Schedule this guy to respawn in the future

	btVector3 deathPos = entityRigidBody->getCenterOfMassPosition();
	deathPos.setY((deathPos.getY() + 4));
	//srand(time(NULL));
	btTransform currentTrans;
	btVector3 ranVelocity;

	// Disperse the player's flags
	for (auto it = flags->begin(); it != flags->end();)
	{
		Flag* curFlag = (*it);
		curFlag->GetRigidBody()->getMotionState()->getWorldTransform(currentTrans);
		currentTrans.setOrigin(deathPos);
		curFlag->GetRigidBody()->getMotionState()->setWorldTransform(currentTrans);
		curFlag->GetRigidBody()->setCenterOfMassTransform(currentTrans);
		deathPos.setY((deathPos.getY() + 4));
		std::pair<int, int> vel = EntitySpawner::getRandomLoc();
		ranVelocity = btVector3((vel.first % 100), (rand() % 100), (vel.second % 100));

		// add Flag to world
		curWorld->addRigidBody(curFlag->GetRigidBody());
		curFlag->GetRigidBody()->setLinearVelocity(ranVelocity);
		// add Flag to client
		btQuaternion quat = curFlag->GetEntityRotation();
		PosInfo out;
		out.cid = ClassId::FLAG;
		out.oid = curFlag->GetObjectId();
		out.x = deathPos.getX();
		out.y = deathPos.getY();
		out.z = deathPos.getZ();
		out.rotw = quat.getW();
		out.rotx = quat.getX();
		out.roty = quat.getY();
		out.rotz = quat.getZ();
		// add Flag to Entity Map
		ServerGame::instance()->sendSpawnPacket(out);
		EntitySpawner::instance()->AddEntity(curFlag->GetClassId(), curFlag->GetObjectId(), curFlag);

		// erase flag from player
		it = flags->erase(it);
		ServerGame::instance()->DecScore(teamId, 1);

	}
	//flags->clear(); //Actually calls delete on flags... didn't seem to correctly work for bullet deletion anways... maybe cause of void pointer
	//LoseFlags();
	//ServerGame::instance()->sendScorePacket();
}

void Player::Move(btVector3* changeVelocity)
{
	if (!alive)
		return;

	Entity::Move(changeVelocity);
}
/*
void Player::SetEntityRotation(float x, float y, float z, float w)
{
	if (!alive)
		return;
	Entity::SetEntityRotation(x, y, z, w);
}*/

