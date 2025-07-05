#pragma once

#include "gl.h"
#include <glm.hpp>
#include "Grid.h"
#include "Shader.h"
#include "Transform.h"
#include "Utility.h"
#include "Buffer.h"
#include "Material.h"
#include "Texture.h"
#include <memory>




class Cube {
private:
    bool m_isTextured;
    Grid* m_parentGrid;
    Transform m_transform;
    glm::vec4 m_color;

    glm::vec3 velocity = glm::vec3(0.0f);
    float mass = 1.0f;
    bool isStatic = false;
    Buffer m_buffer;
    Buffer m_edgeBuffer;
    Texture m_texture;
    Material m_material;
    bool m_edgesInitialized = false;

public:
    Cube(const std::string& textureFilename, Grid* parentGrid = nullptr);
    ~Cube();

    // Methods from original Object
    bool IsTextured() const;
    void IsTextured(bool isTextured);
    Transform& GetTransform();
    const glm::vec4& GetColor() const;

    // Methods from original Cube
    void SetColor(const glm::vec4& color);
    void Render(const Shader& shader);
    bool IntersectsRay(const Utility::Ray& ray, float& t);
    void RenderAABBEdges(const Shader& shader);

    // Physics getters and setters
    glm::vec3 GetVelocity() const { return velocity; }
    void SetVelocity(const glm::vec3& v) { velocity = v; }
    float GetMass() const { return mass; }
    void SetMass(float m) { mass = m > 0 ? m : 1.0f; }
    bool IsStatic() const { return isStatic; }
    void SetStatic(bool s) { isStatic = s; }

};