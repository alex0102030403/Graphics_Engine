#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 lifetimeVec;

out vec3 worldPos;
out float lifetime;

void main() {
    worldPos = position;
    lifetime = lifetimeVec.x; // Normalized lifetime is in the x component
}