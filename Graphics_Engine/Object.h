#pragma once

#include "gl.h"
#include <glm.hpp>
#include "Grid.h"
#include "Shader.h"
#include "Transform.h"
#include "Utility.h"

class Object {
public:
    Object(Grid* parentGrid = nullptr);
    virtual ~Object() = 0 {}

    bool IsTextured() const;
    void IsTextured(bool isTextured);
    Transform& GetTransform();
    const glm::vec4& GetColor() const;

    virtual void Render(const Shader& shader);
    virtual void SetColor(const glm::vec4& color) = 0;

    // New methods for picking
    virtual bool IntersectsRay(const Utility::Ray& ray, float& t) = 0;
    virtual void RenderAABBEdges(const Shader& shader) = 0;

protected:
    bool m_isTextured;
    Grid* m_parentGrid;
    glm::vec4 m_color;
    glm::mat3 m_normal;
    Transform m_transform;
};