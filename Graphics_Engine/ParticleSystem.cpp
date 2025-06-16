#include "ParticleSystem.h"
#include <vector>
#include <cstdlib>
#include <glm.hpp>
#include <iostream>

ParticleSystem::ParticleSystem() : m_maxParticles(0) {}

ParticleSystem::~ParticleSystem() {
    m_buffer.DestroyBuffer();
    m_texture.Unload();
}

bool ParticleSystem::Initialize(size_t maxParticles, const std::string& textureFilename) {
    m_maxParticles = maxParticles;
    m_particles.resize(maxParticles);
    for (auto& particle : m_particles) {
        particle.active = false;
    }
    m_buffer.CreateBuffer(maxParticles, false);
    if (!m_texture.Load(textureFilename)) {
        return false;
    }
    // Initialize VBO data with zeros
    std::vector<float> positions(maxParticles * 3, 0.0f);
    std::vector<float> lifetimes(maxParticles * 4, 0.0f);
    m_buffer.FillVBO(Buffer::VBOType::VertexBuffer, positions.data(), positions.size() * sizeof(float), Buffer::FillType::Ongoing);
    m_buffer.FillVBO(Buffer::VBOType::ColorBuffer, lifetimes.data(), lifetimes.size() * sizeof(float), Buffer::FillType::Ongoing);
    return true;
}

void ParticleSystem::Update(float deltaTime) {
    for (auto& particle : m_particles) {
        if (particle.active) {
            particle.position += particle.velocity * deltaTime;
            particle.lifetime -= deltaTime;
            if (particle.lifetime <= 0.0f) {
                particle.active = false;
            }
        }
    }
    // Prepare VBO data
    std::vector<float> positions(m_maxParticles * 3);
    std::vector<float> lifetimes(m_maxParticles * 4);
    for (size_t i = 0; i < m_maxParticles; ++i) {
        const auto& particle = m_particles[i];
        if (particle.active) {
            positions[i * 3 + 0] = particle.position.x;
            positions[i * 3 + 1] = particle.position.y;
            positions[i * 3 + 2] = particle.position.z;
            float normalizedLifetime = particle.lifetime / particle.initialLifetime;
            lifetimes[i * 4 + 0] = normalizedLifetime;
            lifetimes[i * 4 + 1] = 0.0f;
            lifetimes[i * 4 + 2] = 0.0f;
            lifetimes[i * 4 + 3] = 0.0f;
        }
        else {
            positions[i * 3 + 0] = 0.0f;
            positions[i * 3 + 1] = 0.0f;
            positions[i * 3 + 2] = 0.0f;
            lifetimes[i * 4 + 0] = 0.0f;
            lifetimes[i * 4 + 1] = 0.0f;
            lifetimes[i * 4 + 2] = 0.0f;
            lifetimes[i * 4 + 3] = 0.0f;
        }
    }
    m_buffer.FillVBO(Buffer::VBOType::VertexBuffer, positions.data(), positions.size() * sizeof(float), Buffer::FillType::Ongoing);
    m_buffer.FillVBO(Buffer::VBOType::ColorBuffer, lifetimes.data(), lifetimes.size() * sizeof(float), Buffer::FillType::Ongoing);
}

void ParticleSystem::Render(const Shader& shader, const Camera& camera) {
    shader.SendData("view", camera.GetViewMatrix());
    shader.SendData("projection", camera.GetProjectionMatrix());
    // Set texture
    glActiveTexture(GL_TEXTURE0);
    m_texture.Bind();
    shader.SendData("particleTexture", 0);
    // Link VBOs
    m_buffer.LinkVBO(shader, "position", Buffer::VBOType::VertexBuffer, Buffer::ComponentType::XYZ, Buffer::DataType::FloatData);
    m_buffer.LinkVBO(shader, "lifetimeVec", Buffer::VBOType::ColorBuffer, Buffer::ComponentType::RGBA, Buffer::DataType::FloatData);
    // Render
    m_buffer.Render(Buffer::DrawType::Points);
}

void ParticleSystem::SpawnExplosion(const glm::vec3& position, size_t numParticles, float radius, float speed, float lifetime) {
    size_t spawned = 0;
    for (auto& particle : m_particles) {
        if (!particle.active) {
            // Generate random direction
            float theta = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.1415926535f;
            float phi = acos(2.0f * static_cast<float>(rand()) / RAND_MAX - 1.0f);
            glm::vec3 dir = glm::vec3(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
            // Set particle properties
            particle.position = position + dir * (static_cast<float>(rand()) / RAND_MAX * radius);
            particle.velocity = dir * speed;
            particle.lifetime = lifetime;
            particle.initialLifetime = lifetime;
            particle.active = true;
            spawned++;
			
            if (spawned >= numParticles) {
                break;
            }
        }
    }
}