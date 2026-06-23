#version 410 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
out vec4 frag_color;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform float ambientStrength;

uniform float hWater; uniform float hSand;
uniform float hGrass; uniform float hRock;

uniform sampler2D texWater; uniform sampler2D texSand;
uniform sampler2D texGrass; uniform sampler2D texRock;
uniform sampler2D texSnow;

uniform float u_time; 

void main() {
    //  ANIMAZIONE DELL'ACQUA
    vec2 waterUV = TexCoords;
    
    // Creiamo un effetto onda  basato sul tempo
    waterUV.x += sin(waterUV.y * 20.0 + u_time * 2.0) * 0.01;
    waterUV.y += cos(waterUV.x * 20.0 + u_time * 1.5) * 0.01;
    
    // corrente in diagonale
    waterUV += vec2(u_time * 0.02, u_time * 0.015);

    // 2. Lettura Texture 
    vec3 cWater = texture(texWater, waterUV).rgb;
    vec3 cSand  = texture(texSand, TexCoords).rgb;
    vec3 cGrass = texture(texGrass, TexCoords).rgb;
    vec3 cRock  = texture(texRock, TexCoords).rgb;
    vec3 cSnow  = texture(texSnow, TexCoords).rgb;

    vec3 terrainColor;
    float y = FragPos.y;

    if (y < hWater) {
        terrainColor = cWater;
    } else if (y < hSand) {
        terrainColor = mix(cWater, cSand, smoothstep(hWater, hSand, y));
    } else if (y < hGrass) {
        terrainColor = mix(cSand, cGrass, smoothstep(hSand, hGrass, y));
    } else if (y < hRock) {
        terrainColor = mix(cGrass, cRock, smoothstep(hGrass, hRock, y));
    } else {
        terrainColor = mix(cRock, cSnow, smoothstep(hRock, hRock + 5.0, y));
    }

    vec3 ambient = ambientStrength * lightColor;
    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(lightDir);
    float diff = max(dot(norm, lightDirNorm), 0.0);
    
    vec3 result = (ambient + diff * lightColor) * terrainColor;
    frag_color = vec4(result, 1.0);
}