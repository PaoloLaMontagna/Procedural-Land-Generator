#version 410 core
layout (location = 0) in vec3 pos;

out vec3 TexCoords;
uniform mat4 vp; 

void main() {
    TexCoords = pos;
    vec4 posOut = vp * vec4(pos, 1.0);
    gl_Position = posOut.xyww; 
}