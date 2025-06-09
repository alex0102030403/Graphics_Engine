#include <glm.hpp>
#include "RigidBody.h"
#include "Collider.h"

class BoxCollider : public Collider {
public:
    glm::vec3 halfExtents;
    BoxCollider(const glm::vec3& he) : halfExtents(he) {}
    glm::vec3 getAABBMin(const RigidBody* body) const override {
        glm::mat3 R = glm::mat3_cast(body->orientation);
        glm::vec3 min;
        for (int i = 0; i < 3; ++i) {
            float extent = 0;
            for (int j = 0; j < 3; ++j) {
                extent += halfExtents[j] * std::abs(R[j][i]);
            }
            min[i] = body->position[i] - extent;
        }
        return min;
    }
    glm::vec3 getAABBMax(const RigidBody* body) const override {
        glm::mat3 R = glm::mat3_cast(body->orientation);
        glm::vec3 max;
        for (int i = 0; i < 3; ++i) {
            float extent = 0;
            for (int j = 0; j < 3; ++j) {
                extent += halfExtents[j] * std::abs(R[j][i]);
            }
            max[i] = body->position[i] + extent;
        }
        return max;
    }
};