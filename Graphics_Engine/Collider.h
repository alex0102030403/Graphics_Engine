#pragma once
#include <glm.hpp>
#include "RigidBody.h"

class Collider {
public:
    virtual ~Collider() {}
    virtual glm::vec3 getAABBMin(const RigidBody* body) const = 0;
    virtual glm::vec3 getAABBMax(const RigidBody* body) const = 0;
};