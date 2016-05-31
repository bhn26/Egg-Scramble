#pragma once
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <vector>
#include <map>

#include "Objects/Entity.h"
#include "Objects/CubeMap.h"
#include "Objects/Ground.h"
#include "Objects/StaticObject.h"
#include "Objects/Grass.h"
#include "Objects/InstanceObject.h"
#include "ShaderManager.h"
#include "client\SpriteRenderer.h"
#include "../network/GameData.h"


class Camera;
class Player;

struct PointLight;
class ChickenAnim;

class Scene
{
    float lastTime;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<PointLight> pLight;
	std::unique_ptr<CubeMap> cubeMap;
	std::unique_ptr<Ground> ground;
	std::unique_ptr<InstanceObject> grass;
	std::unique_ptr<InstanceObject> pumpkin;

    Player* player;

    static const int WIDTH;
    static const int HEIGHT;

	std::map<std::pair<int, int>, std::unique_ptr<Entity> > entities;
	std::vector<std::unique_ptr<StaticObject> > static_objects;

    Scene();
    void Setup();

public:
    std::shared_ptr<Shader> basicShader;
    std::shared_ptr<Shader> diffuseShader;
    std::shared_ptr<Shader> modelShader;
    std::shared_ptr<Shader> cubeMapShader;
    std::shared_ptr<Shader> instanceShader;

	static SpriteRenderer * sprite_renderer;

    static Scene* Instance()
    {
        static Scene* instance = new Scene();
        return instance;
    }

    static void Initialize() { Instance()->Setup(); }

	void AddEntity(int cid, int oid, std::unique_ptr<Entity> ent);
	void AddEntity(PosInfo p);
	void RemoveEntity(int cid, int oid);
	std::unique_ptr<Entity>& GetEntity(int cid, int oid);

    void Update();

    void Draw();

    // Interface to camera
    glm::mat4 GetViewMatrix();
    glm::vec3 GetCameraPosition();
    glm::mat4 GetPerspectiveMatrix();

    std::unique_ptr<PointLight>& GetPointLight() { return pLight; }
	Player*& GetPlayer() { return player; }
	StaticObject* GetStaticObject(int i) { return static_objects[i].get(); }
	int GetSize() { return static_objects.size(); }

	// helpers 
	static glm::vec2 Get2D(glm::vec3 coords, glm::mat4 view, glm::mat4 projection/*perspective matrix */, int width, int height);
};
