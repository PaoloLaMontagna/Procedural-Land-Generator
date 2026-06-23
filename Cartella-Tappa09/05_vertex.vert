#version 410 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 uv;

uniform mat4 vp;
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main() {
    gl_Position = vp * vec4(pos, 1.0);
    FragPos = pos;
    Normal = norm;
    TexCoords = uv;
}