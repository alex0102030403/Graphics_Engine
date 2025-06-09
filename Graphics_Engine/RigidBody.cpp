#include "RigidBody.h" // Include the corresponding header file
#include <cmath> // For std::pow

// The glm includes are usually handled by the header, but it doesn't hurt to include them here too if methods directly use them
#include <glm.hpp>
#include <gtc/quaternion.hpp>
#include <gtc/matrix_transform.hpp>

// Constructor definition
RigidBody::RigidBody() :
    position(glm::vec3(0, 0, 0)),
    orientation(glm::quat(1, 0, 0, 0)),
    linearVelocity(glm::vec3(0, 0, 0)),
    angularVelocity(glm::vec3(0, 0, 0)),
    inverseMass(0),
    linearDamping(0.99f),
    angularDamping(0.99f),
    collider(nullptr) // Initialize collider to nullptr
{
}

// Method definitions
void RigidBody::setMass(float mass) {
    inverseMass = mass > 0 ? 1.0f / mass : 0;
}

float RigidBody::getMass() const {
    return inverseMass > 0 ? 1.0f / inverseMass : 0;
}

void RigidBody::setInertiaTensor(const glm::mat3& inertiaTensor) {
    inverseInertiaTensorLocal = glm::inverse(inertiaTensor);
}

void RigidBody::addForce(const glm::vec3& force) {
    forceAccum += force;
}

void RigidBody::addTorque(const glm::vec3& torque) {
    torqueAccum += torque;
}

void RigidBody::addForceAtPoint(const glm::vec3& force, const glm::vec3& point) {
    glm::vec3 r = point - position;
    addForce(force);
    addTorque(glm::cross(r, force));
}

void RigidBody::clearAccumulators() {
    forceAccum = glm::vec3(0, 0, 0);
    torqueAccum = glm::vec3(0, 0, 0);
}

void RigidBody::integrate(float dt) {
	if (inverseMass <= 0) return; // No integration for static bodies
    // Linear motion
    glm::vec3 acceleration = forceAccum * inverseMass;
    linearVelocity += acceleration * dt;
    linearVelocity *= std::pow(linearDamping, dt);
    position += linearVelocity * dt;

    // Angular motion
    glm::mat3 rotationMatrix = glm::mat3_cast(orientation);
    inverseInertiaTensorWorld = rotationMatrix * inverseInertiaTensorLocal * glm::transpose(rotationMatrix);
    glm::vec3 angularAcceleration = inverseInertiaTensorWorld * torqueAccum;
    angularVelocity += angularAcceleration * dt;
    angularVelocity *= std::pow(angularDamping, dt);
    glm::quat omega(0, angularVelocity);
    glm::quat q_dot = 0.5f * omega * orientation;
    orientation += q_dot * dt;
    orientation = glm::normalize(orientation);

    calculateDerivedData();
    clearAccumulators();
}

void RigidBody::calculateDerivedData() {
	orientation = glm::normalize(orientation); // Ensure orientation is normalized
    glm::mat3 rotationMatrix = glm::mat3_cast(orientation);
    inverseInertiaTensorWorld = rotationMatrix * inverseInertiaTensorLocal * glm::transpose(rotationMatrix);
    transformMatrix = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(orientation);
}

glm::mat4 RigidBody::getTransformMatrix() const {
    return transformMatrix;
}

void RigidBody::applyImpulse(const glm::vec3& impulse) {
    if (inverseMass > 0) {
        linearVelocity += impulse * inverseMass;
    }
}