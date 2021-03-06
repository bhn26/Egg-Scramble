#include "Player.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Graphics/Camera.h"
#include "Graphics/Objects/Chicken.h"
#include "Graphics/Shader.h"
#include "Graphics/Scene.h"
#include "Graphics/Lights.h"
#include "Graphics/Model.h"
#include "Graphics/Animation/AnimatedModel.h"
#include "client/ClientGame.h"
#include "Basic/Utils.h"

//// rendering text about the player
#include "client/Window.h"
#include "client/TextRenderer.h"

////////////////////////////////////////////////////////////////////////////////
Player::Player(float x, float y, float z, float rotW, float rotX, float rotY, float rotZ) :
    Entity(glm::vec3(x,y,z), glm::vec3(0.01f)), camAngle(0.0f), modelFile("assets/chickens/objects/chicken.obj"),
    defaultCamFront(glm::normalize(glm::vec3(0.05f, -0.20f, 0.97f))), m_distanceThreshhold_t(1.0f)
{
    //info_panel = new Texture(GL_TEXTURE_2D, "assets/ui/player_info_panel.png");

    SetRelativeCamPosition(glm::vec3(-2.5f, 4.5f, -7.0f));
    camera = std::unique_ptr<Camera>(new Camera(Position() + relativeCamPosition));
    Entity::RotateTo(rotW, rotX, rotY, rotZ);

    // Setup the animated model
    m_model = std::unique_ptr<Animation::AnimatedModel>(new Animation::AnimatedModel);
    m_model->FBXLoadClean("assets/chickens/chicken_dance.fbx", true, "dance");
    m_model->AddAnimation("assets/chickens/chicken_walk.fbx", true, "walk");
    m_model->AddAnimation("assets/chickens/chicken_attack.fbx", false, "melee");
    m_model->AddAnimation("assets/chickens/chicken_peck.fbx", false, "peck");
    m_model->AddAnimation("assets/chickens/chicken_jump.fbx", false, "jump");
    m_model->AddAnimation("assets/chickens/chicken_death.fbx", false, "death");

    m_model->RegisterListener(this);

    Animation::DirectionalLight m_directionalLight;
    m_directionalLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
    m_directionalLight.ambientIntensity = 0.55f;
    m_directionalLight.diffuseIntensity = 0.9f;
    m_directionalLight.direction = glm::vec3(1.0f, 0.0, 0.0);

    Animation::SkinningTechnique* skinTechnique = m_model->GetMesh().GetSkinningTechnique();

    skinTechnique->Enable();

    skinTechnique->SetColorTextureUnit(0); // color texture unit index = 0
    skinTechnique->SetDirectionalLight(*Scene::Instance()->GetDirectionalLight());
    //skinTechnique->SetDirectionalLight(m_directionalLight);
    skinTechnique->SetMatSpecularIntensity(0.0f);
    skinTechnique->SetMatSpecularPower(0);

    m_model->InitBones0();  // Initialize bones to 0 time spot
	alive = true;
	health = 100;
	weapon = -1;
}

////////////////////////////////////////////////////////////////////////////////
Player::Player(int client_id) : Player()
{
    obj_id = client_id;
}

////////////////////////////////////////////////////////////////////////////////
Player::~Player()
{
    //delete info_panel;
}

////////////////////////////////////////////////////////////////////////////////
void Player::SetModelFile(std::string fileName){
    modelFile = fileName;
    //model = std::unique_ptr<Model>(new Model(modelFile.c_str()));
    //m_model = std::unique_ptr<Animation::AnimatedModel>(new Animation::AnimatedModel(fileName));
}

////////////////////////////////////////////////////////////////////////////////
void Player::Update(float deltaTime)
{
    if (m_state == State::WALK)
    {
        if (Utils::CurrentTime() - m_lastTime_t > 0.3f)
        {
            if (DistanceFromLastPos(Position()) < m_distanceThreshhold_t)
                ChangeState(State::IDLE);
            m_lastTime_t = Utils::CurrentTime();
        }
        glm::vec3 position = Position();
        m_lastPos_t.x = position.x;
        m_lastPos_t.y = position.y;
        m_lastPos_t.z = position.z;
        m_lastPos_t = Position();
    }
    m_model->Update(deltaTime);
}

////////////////////////////////////////////////////////////////////////////////
void Player::Draw() const
{
    // Use the appropriate shader (depth or model)
    UseShader();
    // Draw the loaded model
    m_model->Draw(!Scene::Instance()->IsRenderingDepth());

	////////////// DRAW SCORE /////////////////////////
	/*glm::vec2 screen_coords = Scene::Get2D(Position(),
		Scene::Instance()->GetViewMatrix(),
		Scene::Instance()->GetPerspectiveMatrix(),
		Window::width, Window::height);
	//Scene::sprite_renderer->DrawSprite(*info_panel, glm::vec2(screen_coords.x - 100, screen_coords.y - 400), glm::vec2(info_panel->Width(), info_panel->Height()), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));

	char score[5];
	strcpy_s(score, "[");
	strcat_s(score, std::to_string(num_eggs).c_str());
	strcat_s(score, "]");
	TextRenderer::RenderText(score, screen_coords.x, screen_coords.y - 400, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));*/
}

void Player::UseShader() const
{
    Animation::SkinningTechnique* skinTechnique = m_model->GetMesh().GetSkinningTechnique();
    skinTechnique->Enable(); // use shader
    // For rendering Depth, only need model
    if (Scene::Instance()->IsRenderingDepth())
    {
        skinTechnique->SetWorldMatrix(toWorld);
        skinTechnique->SetRenderingDepth(true);
        skinTechnique->SetLightSpaceMatrix(Scene::Instance()->LightSpaceMatrix());
    }
    else
    {
        this->SetShaderUniforms();
    }
}

////////////////////////////////////////////////////////////////////////////////
void Player::SetShaderUniforms() const
{
    Animation::SkinningTechnique* skinTechnique = m_model->GetMesh().GetSkinningTechnique();
    skinTechnique->Enable(); // use shader

    skinTechnique->SetDepthMap("shadowMap", Scene::Instance()->ShadowMapIndex(), Scene::Instance()->DepthMap());
    skinTechnique->SetRenderingDepth(false);

    skinTechnique->SetEyeWorldPos(Scene::Instance()->GetCameraPosition());
    skinTechnique->SetWVP(Scene::Instance()->GetPerspectiveMatrix() * Scene::Instance()->GetViewMatrix() * toWorld);
    skinTechnique->SetWorldMatrix(toWorld);
}

////////////////////////////////////////////////////////////////////////////////
void Player::MoveTo(float x, float y, float z)
{
    if (DistanceFromLastPos(glm::vec3(x,y,z)) < m_distanceThreshhold_t)
    {
        return;
    }
    Entity::MoveTo(x, y, z);
    CalculateCameraPosition();
    CalculateCameraFront();
    if (Scene::Instance()->GetPlayer() == this)
        SetAudioListener();
    if (m_state != State::JUMP)
    {
        ChangeState(State::WALK);
    }
}

////////////////////////////////////////////////////////////////////////////////
void Player::RotateTo(const glm::quat& newOrientation)
{
    glm::mat3 rotation = static_cast<glm::mat3>(glm::quat(newOrientation));
    Entity::RotateTo(rotation);
    CalculateCameraPosition();
    CalculateCameraFront();
    if (Scene::Instance()->GetPlayer() == this)
        SetAudioListener();
}

////////////////////////////////////////////////////////////////////////////////
// Process movement
void Player::ProcessKeyboard(DIRECTION direction, GLfloat deltaTime)
{
    // Update the translation component of the world matrix
    if (direction == D_FORWARD)
        this->toWorld[3] += deltaTime * toWorld[2];
    if (direction == D_BACKWARD)
        this->toWorld[3] -= deltaTime * toWorld[2];
    if (direction == D_LEFT)
        this->toWorld[3] += deltaTime * toWorld[0];
    if (direction == D_RIGHT)
        this->toWorld[3] -= deltaTime * toWorld[0];
    if (direction == D_UP)
        this->toWorld[3] += deltaTime * toWorld[1];
    if (direction == D_DOWN)
        this->toWorld[3] -= deltaTime * toWorld[1];
}

////////////////////////////////////////////////////////////////////////////////
// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Player::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch)
{
    xoffset *= 0.10f;
    yoffset *= 0.03f;

    // Update Front, Right and Up Vectors using the updated Eular angles
    this->toWorld = this->toWorld * glm::rotate(glm::mat4(1.0f), glm::radians(-xoffset), glm::vec3(0.0f, 1.0f, 0.0f));

    camAngle += glm::radians(yoffset);
    const static float pi2 = glm::pi<float>()/2;
    camAngle = (camAngle > pi2) ? pi2 : ((camAngle < -pi2) ? -pi2 : camAngle);

    CalculateCameraPosition();
    CalculateCameraFront();
    SetAudioListener();

	// Don't send me stuff unless you're alive
	if (++tick % 10 == 0 && alive == true)
    {
        ClientGame::instance()->sendRotationPacket();
        tick = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Player::ProcessViewMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch)
{
    xoffset *= m_HViewSensitivity;
    yoffset *= m_VViewSensitivity;

    // Update Front, Right and Up Vectors using the updated Eular angles
	glm::mat4 temp = this->toWorld * glm::rotate(glm::mat4(1.0f), glm::radians(-xoffset), glm::vec3(1.0f, 1.0f, 1.0f));
	for (int col = 0; col < 3; col++)
		for (int row = 0; row < 3; row++)
			temp[col][row] /= scale[col];
	glm::quat trot = static_cast<glm::quat>(temp);

    this->toWorld = this->toWorld * glm::rotate(glm::mat4(1.0f), glm::radians(-xoffset), glm::vec3(0.0f, 1.0f, 0.0f));

    camAngle += glm::radians(yoffset);
    const static float pi2 = glm::pi<float>()/2;
    camAngle = (camAngle > pi2) ? pi2 : ((camAngle < -pi2) ? -pi2 : camAngle);

    CalculateCameraPosition();
    CalculateCameraFront();
    SetAudioListener();

    if (++tick % 10 == 0)
    {
        ClientGame::instance()->sendRotationPacket();
        tick = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Player::ProcessMouseScroll(GLfloat yoffset)
{
    camera->ProcessMouseScroll(yoffset);
}

////////////////////////////////////////////////////////////////////////////////
glm::vec3 Player::CameraPosition() const
{
    //return glm::vec3(toWorld * glm::rotate(glm::mat4(1.0f), camAngle, glm::vec3(-1.0f, 0.0f, 0.0f)) * glm::vec4(camera->Position(), 1.0f));
    return camera->Position();
}

////////////////////////////////////////////////////////////////////////////////
glm::mat4 Player::GetViewMatrix() const
{
    //return camera->GetViewMatrix() * glm::rotate(glm::mat4(1.0f), camAngle, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::inverse(toWorld);
    return camera->GetViewMatrix();
}

////////////////////////////////////////////////////////////////////////////////
glm::vec3 Player::GetFront() const
{
	return camera->Front();
}

////////////////////////////////////////////////////////////////////////////////
glm::mat4 Player::GetPerspectiveMatrix() const
{
    return camera->GetPerspectiveMatrix();
}

////////////////////////////////////////////////////////////////////////////////
glm::mat3 Player::GetNormalMatrix() const
{
    return this->normalMatrix;
}

////////////////////////////////////////////////////////////////////////////////
float Player::GetCamAngle() const
{
    return this->camAngle;
}

////////////////////////////////////////////////////////////////////////////////
void Player::ChangeState(State state)
{
    if (state == m_state && !(state == State::DANCE || state == State::DEATH || state == State::PECK))
        return;

    CheckStopDanceSound();
    SetState(state);
    switch (state)
    {
        case State::IDLE:
        {
            m_model->SetAnimation("dance");
            m_model->Reset();
            break;
        }
        case State::WALK:
        {
            m_model->PlayAnimation("walk");
            m_lastTime_t = Utils::CurrentTime();
            m_lastPos_t = Position();
            break;
        }
        case State::JUMP:
        {
            m_model->PlayAnimation("jump");
            m_lastTime_t = Utils::CurrentTime();
            m_lastPos_t = Position();
            glm::vec3 position = Position();
            SoundsHandler::SoundOptions options(position.x, position.y, position.z);
            ClientGame::instance()->PlaySound("Jump", options);
            break;
        }
        case State::ATTACK:
        {
            m_model->PlayAnimation("melee");
            glm::vec3 position = Position();
            SoundsHandler::SoundOptions options(position.x, position.y, position.z);     // Play at own position
            ClientGame::instance()->PlaySound("Melee", options);     // Play at own position
            break;
        }
        case State::DANCE:
        {
            m_model->PlayAnimation("dance");
            glm::vec3 position = Position();
            SoundsHandler::SoundOptions options(position.x, position.y, position.z);     // Play at own position
            options._loops = true;
            m_danceSoundIndices.push(ClientGame::instance()->PlaySound("Dance", options));
            break;
        }
        case State::DEATH:
        {
            m_model->PlayAnimation("death");
            glm::vec3 position = Position();
            SoundsHandler::SoundOptions options(position.x, position.y, position.z);     // Play at own position
            ClientGame::instance()->PlaySound("Death", options);
            break;
        }
        case State::PECK:
        {
            m_model->PlayAnimation("peck");
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
int Player::GetWeapon()
{
	return this->weapon;
}

void Player::SetWeapon(int weapon)
{
	this->weapon = weapon;
}

////////////////////////////////////////////////////////////////////////////////
void Player::SetTeam(int team)
{
    team_id = team;
    if (team == 1)
    {
        Material material = Material();
        material._diffuse = glm::vec3(1.0f, 210.0f/255.0f, 77.0f/255.0f);
        m_model->ChangeMaterial(1, material);
    }
}

////////////////////////////////////////////////////////////////////////////////
// AnimationPlayer::Listener
void Player::OnFinish()
{
	if (m_state != State::DEATH)
		ChangeState(State::IDLE);
}

void Player::CheckStopDanceSound()
{
    while (m_danceSoundIndices.size())
    {
        int index = m_danceSoundIndices.top();
        m_danceSoundIndices.pop();
        if (index != -1)
            ClientGame::instance()->StopSound(index);
    }
}

////////////////////////////////////////////////////////////////////////////////
void Player::SetRelativeCamPosition(glm::vec3 relativePos)
{
    relativeCamPosition = relativePos;
    relativeCamPerpendicular = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), relativePos);    // Hardcoded world up
}

////////////////////////////////////////////////////////////////////////////////
void Player::CalculateCameraPosition()
{
    camera->position = Position() + (glm::mat3(this->toWorld)
        * glm::mat3(glm::rotate(glm::mat4(1.0f), this->camAngle, relativeCamPerpendicular))
        * relativeCamPosition) / this->scale;     // Divide by scale
}

////////////////////////////////////////////////////////////////////////////////
void Player::CalculateCameraFront()
{
    camera->front = glm::normalize(glm::mat3(this->toWorld)
                                    * glm::mat3(glm::rotate(glm::mat4(1.0f), this->camAngle, relativeCamPerpendicular))
                                    * defaultCamFront);
    camera->front = glm::normalize(camera->front);
}

////////////////////////////////////////////////////////////////////////////////
float Player::DistanceFromLastPos(glm::vec3 newPosition) const
{
    return glm::distance(m_lastPos_t, newPosition);
}

////////////////////////////////////////////////////////////////////////////////
void Player::SetAudioListener() const
{
    glm::vec3 position = Position();
    glm::vec3 direction = camera->Front();
    sf::Listener::setPosition(position.x, position.y, position.z);
    sf::Listener::setDirection(direction.x, direction.y, direction.z);
}
