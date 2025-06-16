#pragma once
#include <vector>
#include "Contact.h"
#include "BoxCollider.h"

class CollisionDetector {
public:
    std::vector<Contact> detectCollisions(const std::vector<RigidBody*>& bodies) {
        std::vector<Contact> contacts;
        for (size_t i = 0; i < bodies.size(); ++i) {
            for (size_t j = i + 1; j < bodies.size(); ++j) {
                //check if is trigger
                RigidBody* bodyA = bodies[i];
                RigidBody* bodyB = bodies[j];
                BoxCollider* colliderA = dynamic_cast<BoxCollider*>(bodyA->collider);
                BoxCollider* colliderB = dynamic_cast<BoxCollider*>(bodyB->collider);
                if (!colliderA || !colliderB) continue;
                if (checkAABBOverlap(bodyA, colliderA, bodyB, colliderB)) {
                    Contact contact;
                    if (detectOBBCollision(bodyA, colliderA, bodyB, colliderB, contact)) {
                        contacts.push_back(contact);
                    }
                }
            }
        }
        return contacts;
    }

private:
    bool checkAABBOverlap(RigidBody* bodyA, BoxCollider* colliderA, RigidBody* bodyB, BoxCollider* colliderB) {
        glm::vec3 minA = colliderA->getAABBMin(bodyA);
        glm::vec3 maxA = colliderA->getAABBMax(bodyA);
        glm::vec3 minB = colliderB->getAABBMin(bodyB);
        glm::vec3 maxB = colliderB->getAABBMax(bodyB);
        bool overlap = (minA.x <= maxB.x && maxA.x >= minB.x) &&
            (minA.y <= maxB.y && maxA.y >= minB.y) &&
            (minA.z <= maxB.z && maxA.z >= minB.z);
        return overlap;
    }


    bool detectOBBCollision(RigidBody* bodyA, BoxCollider* colliderA, RigidBody* bodyB, BoxCollider* colliderB, Contact& contact) {
        const float EPSILON = 1e-5f;
        glm::vec3 C_A = bodyA->position;
        glm::vec3 C_B = bodyB->position;
        glm::vec3 T = C_B - C_A;
        glm::mat3 R_A = glm::mat3_cast(bodyA->orientation);
        glm::mat3 R_B = glm::mat3_cast(bodyB->orientation);
        glm::vec3 E_A = colliderA->halfExtents;
        glm::vec3 E_B = colliderB->halfExtents;

        // Define the 15 potential separating axes
        std::vector<glm::vec3> axes;
        for (int i = 0; i < 3; ++i) axes.push_back(R_A[i]); // A's face normals
        for (int i = 0; i < 3; ++i) axes.push_back(R_B[i]); // B's face normals
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                glm::vec3 cross = glm::cross(R_A[i], R_B[j]);
                float len = glm::length(cross);
                if (len > EPSILON) axes.push_back(cross / len); // Normalize cross product axes
            }
        }

        float minOverlap = std::numeric_limits<float>::max();
        glm::vec3 minAxis;


        // Debug output for ground collisions
        


        // Test all axes
        for (const auto& axis : axes) {
            float proj = std::abs(glm::dot(T, axis));
            float radius_A = 0.0f;
            for (int i = 0; i < 3; ++i) radius_A += std::abs(glm::dot(R_A[i], axis)) * E_A[i];
            float radius_B = 0.0f;
            for (int i = 0; i < 3; ++i) radius_B += std::abs(glm::dot(R_B[i], axis)) * E_B[i];
            float sum = radius_A + radius_B;
            if (proj > sum + EPSILON) return false; // Separating axis found
            float overlap = sum - proj;
            if (overlap < minOverlap) {
                minOverlap = overlap;
                minAxis = axis;
            }
        }

           

        // Collision detected, set contact data
        contact.contactNormal = (glm::dot(T, minAxis) > 0) ? minAxis : -minAxis;

        // Compute support points
        glm::vec3 support_A = C_A;
        for (int i = 0; i < 3; ++i) {
            float s = glm::dot(R_A[i], -contact.contactNormal) >= 0 ? 1.0f : -1.0f;
            support_A += s * E_A[i] * R_A[i];
        }
        glm::vec3 support_B = C_B;
        for (int i = 0; i < 3; ++i) {
            float s = glm::dot(R_B[i], contact.contactNormal) >= 0 ? 1.0f : -1.0f;
            support_B += s * E_B[i] * R_B[i];
        }
        contact.contactPoint = 0.5f * (support_A + support_B);
        contact.penetration = minOverlap;
        contact.bodyA = bodyA;
        contact.bodyB = bodyB;
        contact.restitution = 0.5f; // Default value, adjust as needed
        contact.friction = 0.3f;   // Default value, adjust as needed

        return true;
    }
};