#pragma once

#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "../client/Window.h"

#include <SOIL/SOIL.h>

struct Material
{
    glm::vec3 _diffuse;
    glm::vec3 _specular;
    glm::vec3 _ambient;
    float _shininess;
    Material() : _diffuse(glm::vec3(1.0f)), _specular(glm::vec3(1.0f)), _ambient(glm::vec3(1.0f)), _shininess(0.0f) {}
    bool Valid() const { return glm::length(_diffuse) > 0.0f; } // || glm::length(_specular) || glm::length(_ambient); }
};

class Texture
{
public:
	Texture(GLenum textureTarget = GL_TEXTURE_2D, const std::string& filename = "");
	~Texture();
    //Texture(GLenum textureTarget = GL_TEXTURE0, const std::string& filename = "");

	Texture(const Texture& rhs) = delete;
	Texture& operator=(Texture& rhs) = delete;
	Texture(Texture&& rhs);
	Texture& operator=(Texture&& rhs);

	bool Load();
	// Texture unit like GL_TEXTURE0
	void Bind(GLenum textureUnit) const;

	int Width() { return width; };
	int Height() { return height; };

	static int GetWindowCenter(int width ) { return Window::width / 2 - width / 2; };

private:
    std::string m_fileName;
    GLenum m_textureTarget;     // What type of texture (probably GL_TEXTURE_2D)
    GLuint m_textureID;

	int width;
	int height;
};
