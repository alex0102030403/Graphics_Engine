#pragma once
#include <vector>
#include <iostream> // For debug output
#include "Contact.h"

class ContactResolver {
public:
    void resolveContacts(std::vector<Contact>& contacts, float dt) {
        for (int i = 0; i < 20; ++i) { // Increased to 20 iterations
            for (auto& contact : contacts) {
                resolveContact(contact, dt);
            }
        }
    }

private:
    void resolveContact(Contact& contact, float dt) {
        RigidBody* bodyA = contact.bodyA;
        RigidBody* bodyB = contact.bodyB;

        // Relative velocity along normal
        glm::vec3 relativeVelocity = bodyB->linearVelocity - bodyA->linearVelocity;
        float vr = glm::dot(relativeVelocity, contact.contactNormal);

        // Skip if separating
        if (vr > 0) return;

        // Baumgarte stabilization (bias for position correction)
        float slop = 0.01f; // Allow small penetration before correction
        float biasFactor = 0.2f; // Increase this (e.g., 0.2 to 0.5) for stronger correction
        float bias = (std::max(0.0f, contact.penetration - slop) / dt) * biasFactor;

        // Calculate impulse
        float denominator = bodyA->inverseMass + bodyB->inverseMass; // Simplified, add inertia terms if needed
        float j = -(1.0f + contact.restitution) * vr + bias;
        j /= denominator;

        if (j > 0) {
            glm::vec3 impulse = j * contact.contactNormal;

            // Apply impulse
            if (bodyA->inverseMass > 0) {
                bodyA->linearVelocity -= impulse * bodyA->inverseMass;
            }
            if (bodyB->inverseMass > 0) {
                bodyB->linearVelocity += impulse * bodyB->inverseMass;
            }

            // Debug output
        }

        // Position correction (direct adjustment)
        float correctionFactor = 0.4f; // Tune this (0.1 to 1.0)
        float correction = std::max(0.0f, contact.penetration - slop) * correctionFactor;
        glm::vec3 correctionVector = contact.contactNormal * correction;
        if (bodyA->inverseMass > 0) {
            bodyA->position -= correctionVector * bodyA->inverseMass;
        }
        if (bodyB->inverseMass > 0) {
            bodyB->position += correctionVector * bodyB->inverseMass;
        }
    }
};