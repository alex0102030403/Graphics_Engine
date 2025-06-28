#version 460

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 worldPos[];
in float lifetime[];

out vec2 texCoord;
out float fragLifetime;

uniform mat4 view;
uniform mat4 projection;
uniform float particleSize;

void main() {
    if (lifetime[0] > 0.0) {
        vec4 viewPos = view * vec4(worldPos[0], 1.0);
        float size = particleSize;

        // Bottom-left vertex
        gl_Position = projection * (viewPos + vec4(-size, -size, 0.0, 0.0));
        texCoord = vec2(0.0, 0.0);
        fragLifetime = lifetime[0];
        EmitVertex();

        // Bottom-right vertex
        gl_Position = projection * (viewPos + vec4(size, -size, 0.0, 0.0));
        texCoord = vec2(1.0, 0.0);
        fragLifetime = lifetime[0];
        EmitVertex();

        // Top-left vertex
        gl_Position = projection * (viewPos + vec4(-size, size, 0.0, 0.0));
        texCoord = vec2(0.0, 1.0);
        fragLifetime = lifetime[0];
        EmitVertex();

        // Top-right vertex
        gl_Position = projection * (viewPos + vec4(size, size, 0.0, 0.0));
        texCoord = vec2(1.0, 1.0);
        fragLifetime = lifetime[0];
        EmitVertex();

        EndPrimitive();
    }
}