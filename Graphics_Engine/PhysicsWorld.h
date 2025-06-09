#pragma once
#include <vector>
#include "RigidBody.h"
#include "CollisionDetector.h"
#include "ContactResolver.h"

class PhysicsWorld {
public:
    std::vector<RigidBody*> bodies;
    CollisionDetector* collisionDetector;
    ContactResolver* contactResolver;
    glm::vec3 gravity;
    float fixedDt;

    PhysicsWorld() : gravity(glm::vec3(0, -9.81f, 0)), fixedDt(1.0f / 60.0f) {
        collisionDetector = new CollisionDetector();
        contactResolver = new ContactResolver();
    }
    ~PhysicsWorld() {
        delete collisionDetector;
        delete contactResolver;
        for (auto body : bodies) delete body;
    }

    void addBody(RigidBody* body) { bodies.push_back(body); }
    void setGravity(const glm::vec3& g) { gravity = g; }

    void step(float dt) {
        accumulator += dt;
        while (accumulator >= fixedDt) {
            for (auto body : bodies) {
                if (body->inverseMass > 0) {
                    body->addForce(gravity * (1.0f / body->inverseMass));
                }
            }
            std::vector<Contact> contacts = collisionDetector->detectCollisions(bodies);
            contactResolver->resolveContacts(contacts, fixedDt);
            for (auto body : bodies) {
                body->integrate(fixedDt);
            }
            accumulator -= fixedDt;
        }
    }

    void applyExplosion(const glm::vec3& explosionPos, float radius, float strength) {
        for (auto* body : bodies) {
            if (body->inverseMass > 0) { // Dynamic bodies only
                glm::vec3 toBody = body->position - explosionPos;
                float distance = glm::length(toBody);
                if (distance < radius && distance > 0.001f) { // Avoid division by zero
                    glm::vec3 direction = glm::normalize(toBody);
                    float impulseMagnitude = strength * (1.0f - distance / radius);
                    glm::vec3 impulse = direction * impulseMagnitude;
                    body->applyImpulse(impulse);
                }
            }
        }
    }

private:
    float accumulator = 0.0f;
};