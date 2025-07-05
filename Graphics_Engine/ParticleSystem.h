#pragma once

#include <vector>
#include <string>
#include <glm.hpp>
#include "gl.h"
#include "Shader.h"
#include "Camera.h"
#include "Buffer.h"
#include "Texture.h"

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

    // Initialize the particle system with a maximum number of particles and a texture
    bool Initialize(size_t maxParticles, const std::string& textureFilename);

    // Update particle positions and lifetimes
    void Update(float deltaTime);

    // Render particles using the provided shader and camera
    void Render(const Shader& shader, const Camera& camera);

    // Spawn an explosion of particles at a given position
    void SpawnExplosion(const glm::vec3& position, size_t numParticles, float radius, float speed, float lifetime);

private:
    struct Particle {
        glm::vec3 position;      
        glm::vec3 velocity;      
        float lifetime;          
        float initialLifetime;   
        bool active;             
    };

    std::vector<Particle> m_particles; // Collection of particles
    size_t m_maxParticles;             // Maximum number of particles
    Buffer m_buffer;                   // Buffer for particle data
    Texture m_texture;                 // Texture for particles
};