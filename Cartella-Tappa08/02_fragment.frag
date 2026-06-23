#version 410 core
in vec3 FragPos;
in vec3 Normal;

out vec4 frag_color;

// Variabili che passeremo dal C++
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform float ambientStrength;

void main() {
    // 1. Luce Ambientale (Illumina anche le valli in ombra)
    vec3 ambient = ambientStrength * lightColor;
    
    // 2. Luce Diffusa (Il Sole)
    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(lightDir); // Direzione VERSO il sole
    
    // Prodotto scalare: se la normale guarda verso il sole, diff è 1 (Luminoso). 
    // Se guarda dall'altra parte, diff è 0 (Ombra).
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Uniamo la luce e la moltiplichiamo per il colore del terreno
    vec3 result = (ambient + diffuse) * objectColor;
    frag_color = vec4(result, 1.0);
}