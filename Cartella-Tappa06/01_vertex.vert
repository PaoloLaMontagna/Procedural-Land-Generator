#version 410 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm; // Ora riceviamo la normale dal C++!

uniform mat4 vp;
out vec3 vertex_normal;

void main() {
    gl_Position = vp * vec4(pos, 1.0);
    vertex_normal = norm; // Passiamo la normale al fragment shader
}