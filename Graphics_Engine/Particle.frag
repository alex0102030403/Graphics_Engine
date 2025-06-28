#version 460


in vec2 texCoord;
in float fragLifetime;

uniform sampler2D particleTexture;

out vec4 color;

void main() {
    vec4 texColor = texture(particleTexture, texCoord);
    color = texColor * vec4(1.0, 1.0, 1.0, fragLifetime); // Fade alpha with lifetime
}