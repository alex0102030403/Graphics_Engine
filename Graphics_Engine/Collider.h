#pragma once
#include <glm.hpp>
#include "RigidBody.h"
#include <string>
#include <iostream>

class Collider {
public:
	bool isTrigger = false; // Flag to indicate if the collider is a trigger
	std::string name; // Name of the collider for identification
    virtual ~Collider() {}
    virtual glm::vec3 getAABBMin(const RigidBody* body) const = 0;
    virtual glm::vec3 getAABBMax(const RigidBody* body) const = 0;
};