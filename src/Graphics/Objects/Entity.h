//
//  Entity.hpp
//  egg scramble
//
//  Created by Phoebe on 4/14/16.
//  Copyright © 2016 sunny side up. All rights reserved.
//
#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <SFML/Audio.hpp>

class Shader;
struct DirectionalLight;

enum POSITION
{
	P_FORWARD,
	P_BACKWARD,
	P_LEFT,
	P_RIGHT,
	P_UP,
	P_DOWN
};

class Entity
{
    sf::Music musicPlayer;      // Remove?
protected:
    glm::mat4 toWorld;
    glm::mat3 normalMatrix;
    glm::vec3 scale;
    GLuint VBO, VAO, EBO;

    std::shared_ptr<Shader> shader;

    int obj_id; // This refers to the specific object
    int class_id; // This refers to the type of object

    void ApplyScale();
    void CalculateNormalMatrix();
    void LoadDirectionalLight(DirectionalLight* dLight) const;
public:
    ////////////////////////////////////////////////////////////////////////////////
    // NOTE: Constructors do not initialize vertex/element buffers, nor shader
    Entity(int oid, int cid);
    Entity(glm::mat4 world = glm::mat4(1.0f), glm::vec3 scale = glm::vec3(1.0f));
    Entity(glm::vec3 pos, glm::vec3 scale = glm::vec3(1.0f));
    Entity(float x, float y, float z, float sx = 1.0f, float sy = 1.0f, float sz = 1.0f);

    ////////////////////////////////////////////////////////////////////////////////
    bool PlaySound(std::string soundFile);

    // Basic Rendering functions
    virtual void Draw() const = 0;
    virtual void Update(float deltaTime) = 0;

    // Methods to modify model matrix (position/view)
    virtual void MoveTo(float x, float y, float z) { MoveTo(glm::vec3(x, y, z));}
    virtual void MoveTo(const glm::vec3& newPosition) { toWorld[3] = glm::vec4(newPosition, 1.0f); CalculateNormalMatrix(); }
    virtual void RotateTo(float w, float x, float y, float z) { RotateTo(glm::quat(w, x, y, z)); }
    virtual void RotateTo(const glm::quat& newOrientation);
    virtual void RotateTo(const glm::mat3& newOrientation);
    void SetScale(glm::vec3 scale);
    void SetScale(float scale) { SetScale(glm::vec3(scale)); }

    virtual void UseShader() const;
    virtual void SetShaderUniforms() const = 0;

    ////////////////////////////////////////////////////////////////////////////////
    // Getters
    glm::quat Orientation() const;

    const glm::mat4& ToWorld() const { return toWorld; }
    const glm::mat3& NormalMatrix() const { return normalMatrix; }
    glm::vec3 Position() const { return glm::vec3(toWorld[3]); }

    int GetClassId() const { return class_id; }
    int GetObjId() const { return obj_id; }
    void SetClassId(int cid) { class_id = cid; }
    void SetObjId(int oid) { obj_id = oid; }

	// Process movement
	void ProcessKeyboard(POSITION position, GLfloat deltaTime);

    std::shared_ptr<Shader>& GetShader() { return shader; }

    // for Players only
    virtual void SetScore(int n) {}
    virtual int GetScore() const { return 0; }

	virtual void SetHealth(int h) { }
	virtual int GetHealth() const { return 0; }

private:
};
