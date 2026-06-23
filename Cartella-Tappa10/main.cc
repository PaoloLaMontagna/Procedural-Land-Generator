#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"

#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <iostream>
#include <optional>
#include <cstdlib>

#include "include/hotshaders.hh"
#include "00_mesh.hh"
#include "dem.hh"

// --- CARICAMENTO TEXTURE 2D  ---
GLuint loadTexture(const std::string& path) {
    sf::Image img;
    if (!img.loadFromFile(path)) return 0;
    img.flipVertically();
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.getSize().x, img.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return tex;
}

/// --- CARICAMENTO SKYBOX DALLA CROCE 
GLuint loadCubemapCross(const std::string& path) {
    sf::Image img;
    if (!img.loadFromFile(path)) {
        std::cerr << "ERRORE: Skybox non trovata!" << std::endl;
        return 0;
    }

    int tile = img.getSize().x / 4; 

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    // Mappiamo dove si trovano le facce nella croce
    struct Face { GLenum target; int x; int y; };
    Face faces[6] = {
        {GL_TEXTURE_CUBE_MAP_POSITIVE_X, 2, 1}, // Destra
        {GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 1}, // Sinistra
        {GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 1, 0}, // Sopra
        {GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 1, 2}, // Sotto
        {GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 3, 1}, // Dietro
        {GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 1, 1}  // Davanti
    };

    for (int i = 0; i < 6; i++) {
        sf::Image faceImg;
        unsigned int uTile = (unsigned int)tile;
        
        // 1. Usiamo resize() e passiamo la dimensione come vettore {w, h}
        faceImg.resize({uTile, uTile});
        
        // 2. Creiamo il rettangolo passando Posizione {x, y} e Dimensione {w, h}
        sf::IntRect rect({faces[i].x * tile, faces[i].y * tile}, {tile, tile});
        
        // 3. Copiamo passando la destinazione come vettore {0u, 0u}
        if (!faceImg.copy(img, {0u, 0u}, rect)) {
            std::cerr << "Errore ritaglio faccia " << i << std::endl;
}
        
        glTexImage2D(faces[i].target, 0, GL_RGBA, tile, tile, 0, GL_RGBA, GL_UNSIGNED_BYTE, faceImg.getPixelsPtr());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

////////////////////
// Skybox Class   //
////////////////////
class Skybox {
private:
    GLuint vao, vbo;
public:
    Skybox() {
        // Le coordinate di un cubo 1x1x1
        float vertices[] = {
            -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
        };
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }
    void draw() {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
};

////////////////////
// Camera Fly     //
////////////////////
class Camera {
private:
    glm::vec3 position, front, up, right, world_up;
    float yaw, pitch, speed, sensitivity;

public:
    Camera () {
        reset();
        speed = 2.0f; sensitivity = 0.2f;
    }

    void reset() {
        position = glm::vec3(0.0f, 150.0f, 150.0f);
        world_up = glm::vec3(0.0f, 1.0f, 0.0f);
        yaw = -90.0f;   
        pitch = -45.0f; 
        update_camera_vectors();
    }

    void process_keyboard(int direction) {
        if (direction == 0) position += front * speed;
        if (direction == 1) position -= front * speed;
        if (direction == 2) position -= right * speed;
        if (direction == 3) position += right * speed;
        if (direction == 4) position += up * speed;    
        if (direction == 5) position -= up * speed;    
    }

    void process_mouse_movement(float dx, float dy) {
        yaw += dx * sensitivity;
        pitch -= dy * sensitivity; 
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
        update_camera_vectors();
    }

    glm::mat4 getViewMatrix() { return glm::lookAt(position, position + front, up); }
    glm::mat4 getProjectionMatrix() { return glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 1000.0f); }

private:
    void update_camera_vectors() {
        glm::vec3 new_front;
        new_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        new_front.y = sin(glm::radians(pitch));
        new_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(new_front);
        right = glm::normalize(glm::cross(front, world_up));
        up    = glm::normalize(glm::cross(right, front));
    }
};

////////////////////
// Scene          //
////////////////////
class Scene {
private:
    std::vector<float> points;
    std::vector<unsigned int> indices;
    GLuint vbo, ebo, vao;

public:
    Scene () { send_arrays_2a3f(); generate(4, 0.05f, 10.0f, 12345, false); }
    ~Scene () { glDeleteVertexArrays (1, &vao); glDeleteBuffers (1, &vbo); glDeleteBuffers (1, &ebo); }

    void generate(int octaves, float frequency, float amplitude, unsigned int seed, bool use_dem) {
        points.clear(); indices.clear();
        Mesh mesh (50, 50, octaves, frequency, amplitude, seed, use_dem); 
        mesh.pack4gpu (points, indices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }

    void draw (bool wireframe) {
        glBindVertexArray(vao);
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        glDrawElements(GL_TRIANGLES, indices.size (), GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

private:
    void send_arrays_2a3f () {
        glGenBuffers (1, &vbo); glBindBuffer (GL_ARRAY_BUFFER, vbo);
        glGenVertexArrays (1, &vao); glBindVertexArray (vao);
        glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); glEnableVertexAttribArray (0);
        glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray (1);
        glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray (2);
        glGenBuffers(1, &ebo); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }
};

void handle (const sf::Event::KeyPressed& key, Shaders& shaders, Shaders& skyShaders, Camera& camera) {
    if (ImGui::GetIO().WantCaptureKeyboard) return;
    if (key.scancode == sf::Keyboard::Scancode::R) camera.reset();
    if (key.scancode == sf::Keyboard::Scancode::Space) {
        shaders.reload ("05_vertex.vert", "05_fragment.frag"); shaders.use ();
        skyShaders.reload("skybox.vert", "skybox.frag");
    }
}

void handle (const sf::Event::MouseMoved* mouse, Camera& camera) {
    float x = mouse->position.x; float y = mouse->position.y;
    static float prev_x = x; static float prev_y = y;
    float dx = x - prev_x; float dy = y - prev_y; 
    prev_x = x; prev_y = y;
    if (!ImGui::GetIO().WantCaptureMouse && sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) camera.process_mouse_movement(dx, dy);
}

//////////
// Main //
//////////
int main() {
    unsigned int window_width = 1200, window_height = 800;
    sf::ContextSettings settings;
    settings.depthBits = 32; settings.stencilBits = 8; settings.antiAliasingLevel = 4;
    settings.attributeFlags = sf::ContextSettings::Attribute::Core;
    settings.majorVersion = 4; settings.minorVersion = 1;

    sf::Window window(sf::VideoMode({window_width, window_height}), "Generatore Procedurale - Tappa 10 (Skybox)", sf::Style::Default, sf::State::Windowed, settings);
    window.setVerticalSyncEnabled (true);
    if (!window.setActive (true) || !gladLoadGL (sf::Context::getFunction) || !ImGui::SFML::Init(window, {(float)window_width, (float)window_height})) return 1;
    ImGui_ImplOpenGL3_Init("#version 410 core");

    // CARICAMENTO SHADER TERRENO E SKYBOX
    Shaders shaders ("05_vertex.vert", "05_fragment.frag");
    Shaders skyboxShaders ("skybox.vert", "skybox.frag");
    
    GLint vp_loc = glGetUniformLocation(shaders.program, "vp");
    GLint sky_vp_loc = glGetUniformLocation(skyboxShaders.program, "vp");
    
    GLint lightDir_loc = glGetUniformLocation(shaders.program, "lightDir");
    GLint lightColor_loc = glGetUniformLocation(shaders.program, "lightColor");
    GLint ambient_loc = glGetUniformLocation(shaders.program, "ambientStrength");
    GLint time_loc = glGetUniformLocation(shaders.program, "u_time");
    
    GLint hWater_loc = glGetUniformLocation(shaders.program, "hWater");
    GLint hSand_loc = glGetUniformLocation(shaders.program, "hSand");
    GLint hGrass_loc = glGetUniformLocation(shaders.program, "hGrass");
    GLint hRock_loc = glGetUniformLocation(shaders.program, "hRock");

    // 2. CARICAMENTO TEXTURE (Usa i file jpg che hai già per i biomi!)
    GLuint texWater = loadTexture("../texture/water.png");
    GLuint texSand  = loadTexture("../texture/sand.png");
    GLuint texGrass = loadTexture("../texture/grass.png");
    GLuint texRock  = loadTexture("../texture/rock.png");
    GLuint texSnow  = loadTexture("../texture/snow.png");
    
    shaders.use();
    glUniform1i(glGetUniformLocation(shaders.program, "texWater"), 0);
    glUniform1i(glGetUniformLocation(shaders.program, "texSand"),  1);
    glUniform1i(glGetUniformLocation(shaders.program, "texGrass"), 2);
    glUniform1i(glGetUniformLocation(shaders.program, "texRock"),  3);
    glUniform1i(glGetUniformLocation(shaders.program, "texSnow"),  4);

    // CARICAMENTO SKYBOX
    GLuint cubemapTexture = loadCubemapCross("../texture/cubemap.png");
    skyboxShaders.use();
    glUniform1i(glGetUniformLocation(skyboxShaders.program, "skybox"), 0);

    Camera camera;
    Scene scene; 
    Skybox skybox;

    glEnable (GL_DEPTH_TEST);
    sf::Clock clock;
    sf::Clock global_clock; 

    bool running = true;
    while (running) {
        while (const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>()) running = false;
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                window_width = resized->size.x; window_height = resized->size.y; glViewport (0, 0, window_width, window_height);
            }
            if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed> ()) handle (*key_pressed, shaders, skyboxShaders, camera);
            if (const auto* mouse = event->getIf<sf::Event::MouseMoved> ()) handle (mouse, camera);
        }

        if (!ImGui::GetIO().WantCaptureKeyboard) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) camera.process_keyboard(0);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) camera.process_keyboard(1);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) camera.process_keyboard(2);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) camera.process_keyboard(3);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) camera.process_keyboard(4);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) camera.process_keyboard(5);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::SFML::Update(sf::Mouse::getPosition(window), {(float)window_width, (float)window_height}, clock.restart());

        ImGui::Begin("Pannello di Controllo");
        static int octaves = 4; static float frequency = 0.05f, amplitude = 10.0f; static unsigned int current_seed = 12345;
        static bool use_dem = false; static bool wireframe = false;
        static float hWater = 1.0f, hSand = 2.0f, hGrass = 6.0f, hRock = 12.0f;
        static glm::vec3 sun_dir = glm::vec3(0.5f, 1.0f, 0.3f), sun_color = glm::vec3(1.0f, 0.95f, 0.8f);
        static float ambient_strength = 0.2f;
        bool changed = false;

        if (ImGui::RadioButton("Procedurale", !use_dem)) { use_dem = false; changed = true; } ImGui::SameLine();
        if (ImGui::RadioButton("Dati Reali (DEM)", use_dem)) { use_dem = true; changed = true; }
        if (ImGui::Button("Genera (Seed)")) { current_seed = std::rand(); changed = true; }
        if (ImGui::SliderInt("Ottave", &octaves, 1, 8)) changed = true;
        if (ImGui::SliderFloat("Freq", &frequency, 0.01f, 0.2f)) changed = true;
        if (ImGui::SliderFloat("Amp", &amplitude, 1.0f, 30.0f)) changed = true;
        ImGui::SliderFloat("L. Acqua", &hWater, -2.0f, 5.0f); ImGui::SliderFloat("L. Sabbia", &hSand, 0.0f, 8.0f);
        ImGui::SliderFloat("L. Erba", &hGrass, 2.0f, 15.0f);  ImGui::SliderFloat("L. Roccia", &hRock, 5.0f, 25.0f);
        ImGui::Checkbox("Wireframe", &wireframe);
        ImGui::SliderFloat3("Dir. Sole", &sun_dir.x, -1.0f, 1.0f); ImGui::ColorEdit3("Col. Sole", &sun_color.x);
        ImGui::SliderFloat("Luce Amb.", &ambient_strength, 0.0f, 1.0f);
        ImGui::End();

        if (changed) scene.generate(octaves, frequency, amplitude, current_seed, use_dem);

        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = camera.getProjectionMatrix();

        // ----------------------------------------
        // DISEGNO IL TERRENO
        // ----------------------------------------
        shaders.use();
        glUniformMatrix4fv(vp_loc, 1, GL_FALSE, &(projection * view)[0][0]);
        
        glUniform3f(lightDir_loc, sun_dir.x, sun_dir.y, sun_dir.z);
        glUniform3f(lightColor_loc, sun_color.x, sun_color.y, sun_color.z);
        glUniform1f(ambient_loc, ambient_strength);
        glUniform1f(time_loc, global_clock.getElapsedTime().asSeconds());
        
        glUniform1f(hWater_loc, hWater); glUniform1f(hSand_loc, hSand);
        glUniform1f(hGrass_loc, hGrass); glUniform1f(hRock_loc, hRock);

        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texWater);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, texSand);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, texGrass);
        glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, texRock);
        glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, texSnow);

        scene.draw(wireframe); 

        // ----------------------------------------
        // DISEGNO LA SKYBOX 
        // ----------------------------------------
        glDepthFunc(GL_LEQUAL); 
        skyboxShaders.use();
        
        // Rimuoviamo la componente di traslazione dalla view matrix del cielo
        glm::mat4 view_skybox = glm::mat4(glm::mat3(view)); 
        glUniformMatrix4fv(sky_vp_loc, 1, GL_FALSE, &(projection * view_skybox)[0][0]);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        skybox.draw();
        
        glDepthFunc(GL_LESS); 

        // ----------------------------------------

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        window.display();
    }

    ImGui_ImplOpenGL3_Shutdown(); ImGui::SFML::Shutdown(); return 0;
}