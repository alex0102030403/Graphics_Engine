#pragma once

#include <string>
#include "gl.h"

class Texture
{

public:

	Texture();

	void Bind() const;
	bool Load(const std::string& filename);
	void Unbind() const;
	void Unload() const;

	static void LoadToTarget(GLuint textureID, GLenum target, const std::string& filename);

private:

	GLuint m_ID;

};