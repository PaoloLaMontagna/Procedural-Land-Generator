#version 410 core
in vec3 FragPos;
in vec3 Normal;

out vec4 frag_color;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform float ambientStrength;

uniform float hWater;
uniform float hSand;
uniform float hGrass;
uniform float hRock;

void main() {
    vec3 waterColor = vec3(0.15, 0.35, 0.70);
    vec3 sandColor  = vec3(0.76, 0.70, 0.50);
    vec3 grassColor = vec3(0.24, 0.53, 0.20);
    vec3 rockColor  = vec3(0.40, 0.40, 0.40);
    vec3 snowColor  = vec3(0.95, 0.95, 0.95);

    vec3 terrainColor;
    float y = FragPos.y;

    if (y < hWater) {
        terrainColor = waterColor;
    } else if (y < hSand) {
        terrainColor = mix(waterColor, sandColor, smoothstep(hWater, hSand, y));
    } else if (y < hGrass) {
        terrainColor = mix(sandColor, grassColor, smoothstep(hSand, hGrass, y));
    } else if (y < hRock) {
        terrainColor = mix(grassColor, rockColor, smoothstep(hGrass, hRock, y));
    } else {
        terrainColor = mix(rockColor, snowColor, smoothstep(hRock, hRock + 5.0, y));
    }

    vec3 ambient = ambientStrength * lightColor;
    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(lightDir);
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * terrainColor;
    frag_color = vec4(result, 1.0);
}