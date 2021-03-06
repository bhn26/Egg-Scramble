
#include "ServerGame.h"
#include "engine/Player.h"
#include <algorithm>

unsigned int ServerGame::client_id; 

ServerGame::ServerGame(void)
{
    // id's to assign clients for our table
    client_id = 0;
	game_started = false;
	
	scores[0] = 0;
	scores[1] = 0;

    // set up the server network to listen 
    network = new ServerNetwork(); 

	engine = new Engine();
}

ServerGame::~ServerGame(void)
{
}


void ServerGame::update()
{
	// get new clients only if the game hasn't started
	if (!game_started)
	{
		if (network->acceptNewClient(client_id))
		{
			printf("client %d has been connected to the server\n", client_id);

			// This will be an INIT_CONNECTION packet
			receiveFromClients();
		}
	}	

	auto curr_time = chrono::high_resolution_clock::now();

	chrono::duration<double, milli> fp_stamp = curr_time - start_time;

	int diff = fp_stamp.count();

	if (game_over == false && eggs_spawned && diff >= 300000)
	{
		game_over = true;
		if (scores[0] == scores[1])
		{
			sendGameOverPacket(-1);
		}
		else if(scores[0] > scores[1])
		{
			sendGameOverPacket(0);
		}
		else
		{
			sendGameOverPacket(1);
		}
	}
	else
	{
		if (game_over == false && eggs_spawned && ((diff % 10000) <= 16))
		{
			sendTimeStampPacket(diff);
		}
	}

	receiveFromClients();

	// Check that all clients are ready

	if (game_started && ready_clients >= client_id)
	{
		if (game_over)
			return;

		if (spawned_clients == ready_clients && !eggs_spawned) {
			total_eggs = 2*ready_clients + 1;
			for (int i = 0; i < total_eggs; i++)
			{
				engine->SpawnRandomFlag();
			}
			eggs_spawned = true;
			Sleep(2000); // should wait for clients to respond
			start_time = chrono::high_resolution_clock::now();
		}
		if(!engine->hasInitialSpawned())
			engine->SendPreSpawn(ready_clients);

		// once eggs has spawned, everything has spawned and we can begin the world cycle
		auto t1 = chrono::high_resolution_clock::now();


		if(eggs_spawned)
			engine->GetWorld()->UpdateWorld();

		auto t2 = chrono::high_resolution_clock::now();

		float thresh = 16.67; // 60
        //float thresh = 33;   // 30 frames
		chrono::duration<double, milli> fp_ms = t2 - t1;
		//("DIFFERENCE: %f\n", fp_ms.count());

		if(thresh - fp_ms.count() < 0)
			printf("TIMING ERROR: %f\n", thresh - fp_ms.count());
		else
		{
			//printf("SLEEPING: %f\n", (thresh - fp_ms.count()));

			Sleep((thresh - fp_ms.count()));

			auto t3 = chrono::high_resolution_clock::now();

			chrono::duration<double, milli> fp_after = t3 - t1;

			//("TOTAL AFTER SLEEP: %f\n", fp_after.count());


		}
	}

}

void ServerGame::receiveFromClients()
{
    Packet packet;

    // go through all clients
    std::map<unsigned int, SOCKET>::iterator iter;

    for(iter = network->sessions.begin(); iter != network->sessions.end(); iter++)
    {
        int data_length = network->receiveData(iter->first, network_data);

        if (data_length <= 0) 
        {
            //no data recieved
            continue;
        }

        int i = 0;
        while (i < (unsigned int)data_length) 
        {

            // Deserialize and check the type of packet
            packet.deserialize(&(network_data[i]));
            
            switch (packet.hdr.packet_type) {

                case INIT_CONNECTION:

                    // offset is i here since we want to parse the packet header
                    // receive it and send it right back
                    receiveInitPacket(i);
                    //sendInitPacket();
                    //sendActionPackets();

                    break;

				case SET_USERNAME:
					receiveNamePacket(i);
					break;

				case JOIN_TEAM:
					receiveJoinPacket(i);
					break;

				case READY_GAME:
					ready_clients++;
					//printf("ready clients: %d\nclient_id: %d\n", ready_clients, client_id);
					break;

				case IND_SPAWN_EVENT:
					receiveIndSpawnPacket(i + sizeof(PacketHeader));
					spawned_clients++;
					break;

				case START_GAME:
					receiveStartPacket(i);
					Sleep(7000);
					sendStartPacket();

					break;

                case MOVE_EVENT:
                    // sends a move packet based on if reception was valid
                    receiveMovePacket(i);
                    break;

				case JUMP_EVENT:
					receiveJumpPacket(i);
					break;

				case DANCE_EVENT:
					receiveDancePacket(i);
					break;

                case V_ROTATION_EVENT:
                    receiveRotationPacket(i + sizeof(PacketHeader));
                    break;

				case ATTACK_EVENT:
					receiveAttackPacket(i);
					break;

				case DISCARD_EVENT:
					receiveDiscardPacket(i);
					break;

                default:

                    printf("error in packet types\n");

                    break;
            }
            i += sizeof(Packet);
        }
    }
}

// Handle new init packet from client
void ServerGame::receiveInitPacket(int offset)
{
    struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset]);
    
    printf("server received init packet from client\n");
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

    Packet packet;
    packet.hdr.packet_type = INIT_CONNECTION;

    // Send back to the client and assign client id
    packet.hdr.receiver_id = client_id;
    packet.hdr.sender_id = SERVER_ID;

    packet.serialize(packet_data);

    network->sendToClient(packet_data, packet_size, client_id);
    printf("server sent init packet to client %d\n", client_id);
	client_id++;

}

// Unused, we send the init back right away in receive
// We actually only want to send this back to the client that we received it from, but assume 1 client for now
void ServerGame::sendInitPacket()
{
    printf("sending init to clients\n");
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

    Packet packet;
    packet.hdr.packet_type = INIT_CONNECTION;

    packet.serialize(packet_data);

	network->sendToClient(packet_data, packet_size, client_id);
}


void ServerGame::receiveJoinPacket(int offset) {
	struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset]);
	
	struct PacketData* dat = (struct PacketData *) &(network_data[offset + sizeof(PacketHeader)]);
	struct PosInfo* pi = (struct PosInfo *) &(dat->buf);

	printf("recieved join packet from %d for %d\n", hdr->sender_id, pi->team_id);

	int client = hdr->sender_id;

	if (team_map.find(client) == team_map.end()) { // if new client send all client team data
		for (std::map<int, int>::iterator it = team_map.begin(); it != team_map.end(); it++) {
			sendJoinPacket(it->first);
		}
	}
	
	printf("(before) team_map size = %d", team_map.size());
	team_map[client] = pi->team_id;
	printf("team_map size = %d", team_map.size());
	sendJoinPacket(client);
};

void ServerGame::sendJoinPacket(int client) {
	const unsigned int packet_size = sizeof(Packet);

		// found
		char packet_data[packet_size];

		Packet packet;
		packet.hdr.packet_type = JOIN_TEAM;

		PosInfo p;
		p.id = client;
		p.team_id = team_map[client];

		printf("sending join packet for client %d in team %d\n", client, p.team_id);

		packet.dat.game_data_id = POS_OBJ;

		p.serialize(packet.dat.buf);
		packet.serialize(packet_data);

		packet.hdr.sender_id = SERVER_ID;

		packet.serialize(packet_data);

		network->sendToAll(packet_data, packet_size);
};

void ServerGame::receiveStartPacket(int offset) {

	sendLoadPacket();

	struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset]);

	printf("recieved start packet from %d\n", hdr->sender_id);
	if (!game_started) {
		printf("initializing world with %d players\n", client_id + 1);
		engine->InitWorld(client_id + 1);
		game_started = true;
	}
	/*else {
		printf("re-initializing world with %d players\n", client_id + 1);
		engine->InitWorld(client_id + 1);
	}*/

	// add player
}

void ServerGame::sendStartPacket() { // will add more later based on generated world
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.hdr.packet_type = START_GAME;

    PosInfo p;
    //p.id = client_id + 1;

    packet.dat.game_data_id = POS_OBJ;

	printf("sending start packet with client_id %d\n", client_id + 1);

    p.serialize(packet.dat.buf);
    packet.serialize(packet_data);

    packet.hdr.sender_id = SERVER_ID;

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}

void ServerGame::sendLoadPacket() {
	Packet packet;
	packet.hdr.packet_type = SERVER_LOADING;

	const unsigned int packet_size = sizeof(Packet);

	char packet_data[packet_size];

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}

void ServerGame::sendReadyToSpawnPacket()
{
	Packet packet;
	packet.hdr.packet_type = READY_TO_SPAWN_EVENT;

	const unsigned int packet_size = sizeof(Packet);

	char packet_data[packet_size];

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}

void ServerGame::receiveIndSpawnPacket(int offset)
{
	struct PacketData* dat = (struct PacketData *) &(network_data[offset]);
	struct PosInfo* pi = (struct PosInfo *) &(dat->buf);

	engine->SpawnRandomPlayer(pi->id, pi->team_id, pi->skin);
}

void ServerGame::sendSpawnPacket(PosInfo pi)
{
    Packet packet;
    packet.hdr.packet_type = SPAWN_EVENT;

    const unsigned int packet_size = sizeof(Packet);

    char packet_data[packet_size];

    packet.dat.game_data_id = POS_OBJ;
	pi.serialize(packet.dat.buf);

    packet.serialize(packet_data);

    network->sendToAll(packet_data, packet_size);

}

void ServerGame::sendRemovePacket(ClassId rem_cid, int rem_oid)
{
	Packet packet;
	packet.hdr.packet_type = REMOVE_EVENT;

	const unsigned int packet_size = sizeof(Packet);

	char packet_data[packet_size];

	packet.dat.game_data_id = REM_OBJ;

	RemInfo r;
	r.rem_cid = rem_cid;
	r.rem_oid = rem_oid;

	r.serialize(packet.dat.buf);

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}

void ServerGame::sendRemovePacket(ClassId rem_cid, int rem_oid, ClassId rec_cid, int rec_oid)
{
	Packet packet;
	packet.hdr.packet_type = REMOVE_EVENT;

	const unsigned int packet_size = sizeof(Packet);

	char packet_data[packet_size];

	packet.dat.game_data_id = REM_OBJ;

	RemInfo r;
	r.rem_cid = rem_cid;
	r.rem_oid = rem_oid;
	r.rec_cid = rec_cid;
	r.rec_oid = rec_oid;

	r.serialize(packet.dat.buf);

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}

void ServerGame::sendDiscardPacket(int id)
{
	Packet packet;
	packet.hdr.packet_type = DISCARD_EVENT;

	const unsigned int packet_size = sizeof(Packet);

	char packet_data[packet_size];

	MiscInfo m;
	m.misc1 = id;

	m.serialize(packet.dat.buf);

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}

void ServerGame::sendRemovePacket(ClassId rem_cid, int rem_oid, ClassId rec_cid, int rec_oid, CollectType c_type, int subtype)
{
	Packet packet;
	packet.hdr.packet_type = REMOVE_EVENT;

	const unsigned int packet_size = sizeof(Packet);

	char packet_data[packet_size];

	packet.dat.game_data_id = REM_OBJ;

	RemInfo r;
	r.rem_cid = rem_cid;
	r.rem_oid = rem_oid;
	r.rec_cid = rec_cid;
	r.rec_oid = rec_oid;
	r.sub_id = c_type;
	r.sub_id2 = subtype;

	r.serialize(packet.dat.buf);

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}

void ServerGame::receiveMovePacket(int offset)
{

	struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset]);
    struct PacketData* dat = (struct PacketData *) &(network_data[offset + sizeof(PacketHeader)]);
    struct PosInfo* pi = (struct PosInfo *) &(dat->buf);
	Entity* ent = (Entity*)(EntitySpawner::instance()->GetEntity(ClassId::PLAYER, hdr->sender_id));

	Player* player = (Player*)(EntitySpawner::instance()->GetEntity(ClassId::PLAYER, hdr->sender_id));

	// don't take client input if stunned
	if(player->GetStun() > 0)
		return;

	switch (pi->direction) 
	{
		case MOVE_FORWARD:
		{
			btVector3 curVec = player->GetEntityVelocity();
			curVec.setZ(player->GetPlayerSpeed());
			curVec.setX(0);
			ent->Move((&curVec));
			break;
		}
		case MOVE_BACKWARD:
		{
			btVector3 curVec = player->GetEntityVelocity();
			curVec.setZ(-(player->GetPlayerSpeed()));
			curVec.setX(0);
			ent->Move((&curVec));
			break;
		}
		case MOVE_LEFT:
		{
			btVector3 curVec = player->GetEntityVelocity();
			curVec.setX(player->GetPlayerSpeed());
			curVec.setZ(0);
			ent->Move((&curVec));
			break;
		}
		case MOVE_RIGHT:
		{
			btVector3 curVec = player->GetEntityVelocity();
			curVec.setX(-(player->GetPlayerSpeed()));
			curVec.setZ(0);
			ent->Move((&curVec));
			break;
		}
	}
	
}

void ServerGame::sendMovePacket(ClassId class_id, int obj_id)
{
		Entity* ent = (Entity*)(EntitySpawner::instance()->GetEntity(class_id, obj_id));

		// If this is not a player and the player is dead, don't update
		if (class_id == ClassId::PLAYER)
		{
			if (!((Player*)ent)->IsAlive())
				return;
		}

        Packet packet;
		packet.hdr.sender_id = SERVER_ID;
        packet.hdr.packet_type = MOVE_EVENT;

        PosInfo p;
        packet.dat.game_data_id = POS_OBJ;
	
		// Extract the vector and send it with the posinfo object
		btVector3 vec = ent->GetEntityPosition();

		p.cid = class_id;
		p.oid = obj_id;
		p.x = vec.getX();
		p.y = vec.getY();
		p.z = vec.getZ();

		if (class_id == PLAYER) {
			p.num_eggs = ((Player*)ent)->GetScore();
			p.jump = ((Player*)ent)->GetJumpSem();
			p.hp = ((Player*)ent)->GetHP();
		}

        p.serialize(packet.dat.buf);

        const unsigned int packet_size = sizeof(Packet);
        char packet_data[packet_size];

        packet.serialize(packet_data);

        network->sendToAll(packet_data, packet_size);
}

void ServerGame::receiveRotationPacket(int offset) {

    struct PacketData *dat = (struct PacketData *) &(network_data[offset]);
    struct PosInfo* pi = (struct PosInfo *) &(dat->buf);

	struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset - sizeof(PacketHeader)]);

	// All rotation packets will be player type, since it's from client
	Entity* ent = (Entity*)(EntitySpawner::instance()->GetEntity(ClassId::PLAYER, hdr->sender_id));

	ent->SetEntityRotation(0, pi->roty, 0, pi->rotw);
	sendRotationPacket(ClassId::PLAYER, hdr->sender_id);
}

void ServerGame::sendRotationPacket(int class_id, int obj_id) {
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

	Entity* ent = (Entity*)(EntitySpawner::instance()->GetEntity(class_id, obj_id));

	// If this is not a player and the player is dead, don't update
	if (class_id == ClassId::PLAYER)
	{
		if (!((Player*)ent)->IsAlive())
			return;
	}

    Packet packet;
	packet.hdr.sender_id = SERVER_ID;
    packet.hdr.packet_type = V_ROTATION_EVENT;

    packet.dat.game_data_id = POS_OBJ;

	PosInfo p;
	p.cid = (ClassId) class_id;
	p.oid = obj_id;

	// why do we switch w and y and multiply by negative, we don't know but it fixes it
	btQuaternion q = ent->GetEntityRotation();
	btVector3 v = ent->GetEntityPosition();
	p.x = v.getX();
	p.y = v.getY();
	p.z = v.getZ();
	p.rotw = q.getW();
	p.rotx = q.getX();
	p.roty = q.getY();
	p.rotz = q.getZ();

    p.serialize(packet.dat.buf);
    
    packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}

void ServerGame::receiveJumpPacket(int offset)
{
    struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset]);

	Player* player = (Player*)(EntitySpawner::instance()->GetEntity(ClassId::PLAYER, hdr->sender_id));

	// ignore client input if stunned
	if(player->GetStun() > 0)
		return;

	player->JumpPlayer();
}

void ServerGame::sendScorePacket() {
	Packet packet;
	packet.hdr.packet_type = UPDATE_SCORE;

	const unsigned int packet_size = sizeof(Packet);

	char packet_data[packet_size];

	packet.dat.game_data_id = SCORE_OBJ;

	ScoreInfo s;
	s.t0_score = scores[0];
	s.t1_score = scores[1];

	printf("sending score packet: %d, %d\n", s.t0_score, s.t1_score);
	s.serialize(packet.dat.buf);

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}

void ServerGame::sendGameOverPacket(int winner) {
	Packet packet;
	packet.hdr.packet_type = GAME_OVER;
	game_over = true;

	const unsigned int packet_size = sizeof(Packet);

	char packet_data[packet_size];

	packet.dat.game_data_id = SCORE_OBJ;

	ScoreInfo s;
	s.t0_score = scores[0];
	s.t1_score = scores[1];

	printf("sending game over\n");
	s.serialize(packet.dat.buf);

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}

void ServerGame::receiveAttackPacket(int offset) {
	struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset]);
	struct PacketData* dat = (struct PacketData *) &(network_data[offset + sizeof(PacketHeader)]);
	struct MiscInfo* m = (struct MiscInfo *) &(dat->buf);
	Player* player = (Player*)(EntitySpawner::instance()->GetEntity(ClassId::PLAYER, hdr->sender_id));
	
	player->SetCamAngle(m->misc3);

	if (m->misc1 == AttackType::WEAPON_ATTACK)
		player->UseWeapon();
	else if (m->misc1 == AttackType::PECK)
		player->UsePeck();
}

void ServerGame::sendTimeStampPacket(int diff)
{
	Packet packet;
	packet.hdr.sender_id = SERVER_ID;
	packet.hdr.packet_type = TIME_EVENT;

	MiscInfo m;
	m.misc1 = (diff / 1000);

	m.serialize(packet.dat.buf);

	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}

void ServerGame::sendAttackPacket(int id) {
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.hdr.sender_id = SERVER_ID;
	packet.hdr.packet_type = ATTACK_EVENT;

	packet.dat.game_data_id = EMOTE_OBJ;

	EmoteInfo e;
	e.id = id;
	e.serialize(packet.dat.buf);

	packet.serialize(packet_data);
	network->sendToAll(packet_data, packet_size);
}

void ServerGame::receiveDiscardPacket(int offset) {
	//struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset]);

	//shared_ptr<Player> player = engine->GetWorld()->GetPlayer(hdr->sender_id);
	struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset]);
	Player* player = (Player*)(EntitySpawner::instance()->GetEntity(ClassId::PLAYER, hdr->sender_id));
	player->DiscardWeapon();
}

void ServerGame::receiveDancePacket(int offset) {
	struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset]);
	sendDancePacket(hdr->sender_id);
}

void ServerGame::sendDancePacket(int id) {
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.hdr.sender_id = SERVER_ID;
	packet.hdr.packet_type = DANCE_EVENT;

	packet.dat.game_data_id = EMOTE_OBJ;

	EmoteInfo e;
	e.id = id;
	e.serialize(packet.dat.buf);

	packet.serialize(packet_data);
	network->sendToAll(packet_data, packet_size);
}

void ServerGame::sendDeathPacket(int id) {
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.hdr.sender_id = SERVER_ID;
	packet.hdr.packet_type = DEATH_EVENT;

	packet.dat.game_data_id = EMOTE_OBJ;

	EmoteInfo e;
	e.id = id;
	e.serialize(packet.dat.buf);

	packet.serialize(packet_data);
	network->sendToAll(packet_data, packet_size);
}

void ServerGame::sendRespawnPacket(int id) {
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.hdr.sender_id = SERVER_ID;
	packet.hdr.packet_type = RESPAWN_EVENT;

	packet.dat.game_data_id = EMOTE_OBJ;

	EmoteInfo e;
	e.id = id;
	e.serialize(packet.dat.buf);

	packet.serialize(packet_data);
	network->sendToAll(packet_data, packet_size);
}

void ServerGame::sendNamePacket(int player_id) {
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.hdr.sender_id = SERVER_ID;
	packet.hdr.packet_type = SET_USERNAME;

	packet.dat.game_data_id = NAME_OBJ;

	NameInfo n;
	n.player_id = player_id;
	n.name = name_map.at(player_id);
	n.serialize(packet.dat.buf);

	packet.serialize(packet_data);
	network->sendToAll(packet_data, packet_size);
}

void ServerGame::receiveNamePacket(int offset) {
	struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset]);
	struct PacketData* dat = (struct PacketData *) &(network_data[offset + sizeof(PacketHeader)]);
	struct NameInfo* n = (struct NameInfo *) &(dat->buf);

	name_map[hdr->sender_id] = n->name;
	/*Player* player = (Player*)(EntitySpawner::instance()->GetEntity(ClassId::PLAYER, hdr->sender_id));
	player->SetName(n->name);*/

	sendNamePacket(hdr->sender_id);

}