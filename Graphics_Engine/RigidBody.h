#pragma once

#include <glm.hpp>
#include <gtc/quaternion.hpp>
#include <gtc/matrix_transform.hpp>
#include <vector> // This might not be strictly needed in the header if not directly used in the public interface or member variables exposed.
#include <cmath>  // This might not be strictly needed in the header.

// Forward declaration of Collider class if it's defined elsewhere.
// If Collider is a simple struct or part of this library, you might include its header here.
class Collider;

class RigidBody {
public:
    // Member variables
    glm::vec3 position;
    glm::quat orientation;
    glm::vec3 linearVelocity;
    glm::vec3 angularVelocity;
    float inverseMass;
    glm::mat3 inverseInertiaTensorLocal;
    glm::vec3 forceAccum;
    glm::vec3 torqueAccum;
    float linearDamping;
    float angularDamping;
    glm::mat3 inverseInertiaTensorWorld;
    glm::mat4 transformMatrix;
    Collider* collider; // Assuming Collider is defined elsewhere

    // Constructor
    RigidBody();

    // Public methods
    void setMass(float mass);
    float getMass() const;
    void setInertiaTensor(const glm::mat3& inertiaTensor);
    void addForce(const glm::vec3& force);
    void addTorque(const glm::vec3& torque);
    void addForceAtPoint(const glm::vec3& force, const glm::vec3& point);
    void clearAccumulators();
    void integrate(float dt);
    void calculateDerivedData();
    void applyImpulse(const glm::vec3& impulse);
    glm::mat4 getTransformMatrix() const;
};