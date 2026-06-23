#version 410 core
in vec3 vertex_normal;
out vec4 frag_color;

void main() {
    // Le normali vanno da -1 a 1. I colori da 0 a 1. 
    // Facciamo una piccola conversione matematica per mappare X,Y,Z su R,G,B!
    vec3 color = vertex_normal * 0.5 + 0.5;
    frag_color = vec4(color, 1.0);
}