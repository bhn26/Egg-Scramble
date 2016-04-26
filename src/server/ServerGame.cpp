
#include "ServerGame.h"

unsigned int ServerGame::client_id; 

ServerGame::ServerGame(void)
{
    // id's to assign clients for our table
    client_id = 0;
	game_started = false;

    // set up the server network to listen 
    network = new ServerNetwork(); 

	engine = new Engine();
}

ServerGame::~ServerGame(void)
{
	delete curWorld;
	delete solv;
	delete pairCache;
	delete disp;
	delete colConfig;
}

void ServerGame::initGameInstance()
{
	btDefaultCollisionConfiguration * collisionConfig = new btDefaultCollisionConfiguration();
	
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfig);

	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();

	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfig);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));

	// Add Ground Object
	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(btScalar(0.), btScalar(1.), btScalar(0.)), 1);

	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
	groundRigidBodyCI.m_friction = .9;
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	ground = groundRigidBody;
	dynamicsWorld->addRigidBody(groundRigidBody);

	curWorld = dynamicsWorld;
	solv = solver;
	pairCache = overlappingPairCache;
	disp = dispatcher;
	colConfig = collisionConfig;
}

void ServerGame::update()
{
    // get new clients
   if(network->acceptNewClient(client_id))
   {
        printf("client %d has been connected to the server\n",client_id);

        // This will be an INIT_CONNECTION packet
        receiveFromClients();
        client_id++;
   }
   else
   {
       receiveFromClients();
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

				case START_GAME:
					receiveStartPacket(i);
					sendStartPacket();

					break;

                case ACTION_EVENT:

                    printf("server received action event packet from client\n");

                    sendActionPackets();

                    break;

                case MOVE_EVENT:
                    // sends a move packet based on if reception was valid
                    receiveMovePacket(i);
                    break;

                case SPAWN_EVENT:

                    // Receive packet
                    // Check validity of event
                    // If check passes, send update to clients
                    receiveSpawnPacket(i);
                    sendSpawnPacket();
              
                    break;

                case V_ROTATION_EVENT:
                    receiveVRotationPacket(i + sizeof(PacketHeader));
                
                    break;

                default:

                    printf("error in packet types\n");

                    break;
            }
            i += sizeof(Packet);
        }
    }
}


void ServerGame::sendActionPackets()
{
    // send action packet
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

    Packet packet;
    packet.hdr.packet_type = ACTION_EVENT;

    packet.serialize(packet_data);

    network->sendToAll(packet_data,packet_size);
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

}

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

void ServerGame::receiveStartPacket(int offset) {
	struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset]);

	printf("recieved start packet from %d\n", hdr->sender_id);
	if (!game_started) {
		printf("initializing world with %d players", client_id + 1);
		engine->InitWorld(client_id + 1);
		game_started = true;
	}

	// add player
}

void ServerGame::sendStartPacket() { // will add more later based on generated world
	printf("sent start packet");
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.hdr.packet_type = START_GAME;
<<<<<<< 849bab3ad8a94e6e1f5e449c2f13e0b7c0efe664
    packet.hdr.sender_id = SERVER_ID;
=======
	packet.pi.id = client_id + 1;
>>>>>>> multiplayer WIP

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}

// Assume one client for now, we're going to need to send client id with each packet probably
// in order to distinguish the owner of the event, right now we don't do this.
void ServerGame::receiveSpawnPacket(int offset)
{
    struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset]);
    
    printf("Received spawn packet from client %d\n", hdr->sender_id);

    // Check if valid spawn and stuff like that
}


void ServerGame::sendSpawnPacket()
{
    Packet packet;
    packet.hdr.packet_type = SPAWN_EVENT;

    const unsigned int packet_size = sizeof(Packet);

<<<<<<< 849bab3ad8a94e6e1f5e449c2f13e0b7c0efe664
    int x = rand() % 5;
    int y = rand() % 5;

    printf("spawned dummy at (%d,%d)\n", x, y);
    world->spawnDummy(x, y);
=======
>>>>>>> multiplayer WIP
    char packet_data[packet_size];

    PosInfo p;
    p.x = x;
    p.y = y;

    packet.dat.obj_id = POS_OBJ;

    p.serialize(packet.dat.buf);
    packet.serialize(packet_data);

    //printf("size of Packet: %d\n", packet_size);

    network->sendToAll(packet_data, packet_size);

}

void ServerGame::receiveMovePacket(int offset)
{
<<<<<<< 849bab3ad8a94e6e1f5e449c2f13e0b7c0efe664
    struct PacketData *dat = (struct PacketData *) &(network_data[offset]);
    struct PosInfo* pi = (struct PosInfo *) &(dat->buf);
=======
	struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset]);
	printf("recieved move packet from %d\n", hdr->sender_id);
    struct PosInfo* pi = (struct PosInfo *) &(network_data[offset + sizeof(PacketHeader)]);

	shared_ptr<Player> player = engine->GetWorld()->GetPlayer(hdr->sender_id);
>>>>>>> multiplayer WIP

    //printf("dummy's current pos is (%d,%d)\n", dpi.x, dpi.y);

	player->Move(pi->direction);

	sendMovePacket(hdr->sender_id);

}

void ServerGame::sendMovePacket(int client)
{
		shared_ptr<Player> player = engine->GetWorld()->GetPlayer(client);
        Packet packet;
        packet.hdr.packet_type = MOVE_EVENT;
<<<<<<< 849bab3ad8a94e6e1f5e449c2f13e0b7c0efe664

        PosInfo p;
        p.direction = direction;

        packet.dat.obj_id = POS_OBJ;

        p.serialize(packet.dat.buf);
=======
		
		packet.pi = player->GetPosition();
		packet.pi.id = client;
>>>>>>> multiplayer WIP

        const unsigned int packet_size = sizeof(Packet);
        char packet_data[packet_size];

        packet.serialize(packet_data);

        network->sendToAll(packet_data, packet_size);
        //printf("Sent move packet to clients\n");
}

void ServerGame::receiveVRotationPacket(int offset) {
<<<<<<< 849bab3ad8a94e6e1f5e449c2f13e0b7c0efe664
    struct PacketData *dat = (struct PacketData *) &(network_data[offset]);
    struct PosInfo* pi = (struct PosInfo *) &(dat->buf);
=======
	struct PacketHeader* hdr = (struct PacketHeader *) &(network_data[offset - sizeof(PacketHeader)]);
	struct PosInfo* pi = (struct PosInfo *) &(network_data[offset]);

	shared_ptr<Player> player = engine->GetWorld()->GetPlayer(hdr->sender_id);
>>>>>>> multiplayer WIP

    // TODO - rotate player in game state
    player->Rotate(pi->v_rotation, pi->h_rotation);

	sendVRotationPacket(hdr->sender_id);
}

void ServerGame::sendVRotationPacket(int client) {
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

	shared_ptr<Player> player = engine->GetWorld()->GetPlayer(client);
    Packet packet;
    packet.hdr.packet_type = V_ROTATION_EVENT;
    packet.hdr.sender_id = client_id;
    packet.hdr.receiver_id = SERVER_ID;

<<<<<<< 849bab3ad8a94e6e1f5e449c2f13e0b7c0efe664
    PosInfo p = world->getDummyRotation();
=======
	packet.pi = player->GetPosition();
	packet.pi.id = client;
>>>>>>> multiplayer WIP

    packet.dat.obj_id = POS_OBJ;

    p.serialize(packet.dat.buf);
    
    packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}