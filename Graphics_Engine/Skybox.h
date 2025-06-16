#pragma once

#include <vector>
#include <string>
#include "gl.h"
#include "Shader.h"
#include <glm.hpp>
#include "Camera.h"

class Skybox {
public:
    Skybox(const std::vector<std::string>& faces);
    ~Skybox();

    void Render(Shader& shader, const Camera& camera);

private:
    GLuint VAO;        // Vertex Array Object
    GLuint VBO;        // Vertex Buffer Object
    GLuint textureID;  // Cube map texture ID

    void SetupSkybox(const std::vector<std::string>& faces);
};