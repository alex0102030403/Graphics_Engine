#include "Skybox.h"
#include <iostream>
#include "Utility.h"

static SDL_Surface* Rotate180(SDL_Surface* surface) {
    if (!surface) return nullptr; 

    SDL_Surface* rotated = SDL_CreateRGBSurface(surface->flags, surface->w, surface->h,
        surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask,
        surface->format->Bmask, surface->format->Amask);
    if (!rotated) {
        return nullptr;
    }

    Uint8* srcPixels = (Uint8*)surface->pixels;
    Uint8* dstPixels = (Uint8*)rotated->pixels;
    int pitch = surface->pitch;
    int height = surface->h;
    int width = surface->w;
    int bpp = surface->format->BytesPerPixel;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int srcX = width - 1 - x;
            int srcY = height - 1 - y;
            Uint8* srcPtr = srcPixels + srcY * pitch + srcX * bpp;
            Uint8* dstPtr = dstPixels + y * pitch + x * bpp;
            memcpy(dstPtr, srcPtr, bpp);
        }
    }

    return rotated;
}

Skybox::Skybox() : VAO(0), VBO(0), IBO(0), indexCount(0), skyShader(nullptr), textureId(0)
{
}

Skybox::Skybox(std::vector<std::string> faceLocations, Shader* shader) : VAO(0), VBO(0), IBO(0), indexCount(0), skyShader(shader), textureId(0)
{
    // Texture Setup
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    int width, height, bitDepth;
    for (size_t i = 0; i < 6; i++)
    {
        SDL_Surface* textureData = IMG_Load(faceLocations[i].c_str());
        if (!textureData)
        {
            std::cout << "Failed to load texture: " << faceLocations[i] << std::endl;
            return;
        }

        if (i == 2 || i == 3) {
            SDL_Surface* rotated = Rotate180(textureData);
            if (rotated) {
                SDL_FreeSurface(textureData);
                textureData = rotated;
            }
        }
        auto width = textureData->w;
        auto height = textureData->h;
        auto* pixels = (Uint8*)textureData->pixels;
        auto depth = textureData->format->BytesPerPixel;
        auto format = ((depth == 4) ? GL_RGBA : GL_RGB);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create the mesh
    CreateSkyboxMesh();
}

void Skybox::CreateSkyboxMesh()
{
    // Define vertices and indices for a cube (simplified example)
    float vertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f
    };


    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,  // Back
        4, 5, 6, 6, 7, 4,  // Front
        0, 1, 5, 5, 4, 0,  // Left
        3, 2, 6, 6, 7, 3,  // Right
        0, 7, 4, 3, 0, 7,  // Top
        1, 2, 6, 6, 5, 1   // Bottom
    };

    indexCount = 36;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &IBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Skybox::RenderSkyboxMesh()
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Skybox::DrawSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    // Remove translation from view matrix (skybox stays centered)
    viewMatrix = glm::mat4(glm::mat3(viewMatrix));
    glDepthMask(GL_FALSE);

    // Send projection and view matrices
    skyShader->SendData("projection", projectionMatrix);
    skyShader->SendData("view", viewMatrix);

    
	
    // Bind cubemap texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
    skyShader->SendData("skybox", 0);  // Assuming "skybox" is the uniform name in your shader

    RenderSkyboxMesh();

    glDepthMask(GL_TRUE);
}

Skybox::~Skybox()
{
    if (IBO != 0)
    {
        glDeleteBuffers(1, &IBO);
    }
    if (VBO != 0)
    {
        glDeleteBuffers(1, &VBO);
    }
    if (VAO != 0)
    {
        glDeleteVertexArrays(1, &VAO);
    }
    if (skyShader != nullptr)
    {
        skyShader->Destroy();
    }
    if (textureId != 0)
    {
        glDeleteTextures(1, &textureId);
    }
}