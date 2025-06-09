#pragma once

#include <glm.hpp>
#include "RigidBody.h"

struct Contact {
    glm::vec3 contactPoint;
    glm::vec3 contactNormal;
    float penetration;
    RigidBody* bodyA;
    RigidBody* bodyB;
    float restitution;
    float friction;

    Contact() : bodyA(nullptr), bodyB(nullptr), restitution(0.5f), friction(0.3f) {}
};