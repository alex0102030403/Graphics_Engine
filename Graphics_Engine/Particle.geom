#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 worldPos[];
in float lifetime[];

out vec2 texCoord;
out float fragLifetime;

uniform mat4 view;
uniform mat4 projection;

void main() {
    if (lifetime[0] > 0.0) {
        vec3 pos = worldPos[0];
        float lt = lifetime[0];

        vec4 viewPos = view * vec4(pos, 1.0);
        float size = 0.1; // Particle size in view space, adjust as needed

        // Vertex 0: bottom-left
        gl_Position = projection * (viewPos + vec4(-size, -size, 0.0, 0.0));
        texCoord = vec2(0.0, 0.0);
        fragLifetime = lt;
        EmitVertex();

        // Vertex 1: bottom-right
        gl_Position = projection * (viewPos + vec4(size, -size, 0.0, 0.0));
        texCoord = vec2(1.0, 0.0);
        fragLifetime = lt;
        EmitVertex();

        // Vertex 2: top-left
        gl_Position = projection * (viewPos + vec4(-size, size, 0.0, 0.0));
        texCoord = vec2(0.0, 1.0);
        fragLifetime = lt;
        EmitVertex();

        // Vertex 3: top-right
        gl_Position = projection * (viewPos + vec4(size, size, 0.0, 0.0));
        texCoord = vec2(1.0, 1.0);
        fragLifetime = lt;
        EmitVertex();

        EndPrimitive();
    }
}