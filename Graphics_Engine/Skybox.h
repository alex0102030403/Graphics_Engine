#pragma once

#include <vector>
#include <string>
#include <SDL_image.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Shader.h"  // Your new Shader class

class Skybox
{
public:
    Skybox();
    Skybox(std::vector<std::string> faceLocations, Shader* shader);
    void DrawSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    ~Skybox();

private:
    void CreateSkyboxMesh();
    void RenderSkyboxMesh();

    GLuint VAO, VBO, IBO;
    GLsizei indexCount;

    Shader* skyShader;  // Pointer to your Shader class
    GLuint textureId;
};