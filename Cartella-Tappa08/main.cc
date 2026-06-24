#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"

#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp> // Serve per caricare le Texture JPG
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

// Funzione per caricare le immagini
GLuint loadTexture(const std::string& path) {
    sf::Image img;
    if (!img.loadFromFile(path)) {
        std::cerr << "ERRORE: Non trovo l'immagine " << path << std::endl;
        return 0;
    }
    img.flipVertically(); // OpenGL vuole l'asse Y capovolto
    
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

////////////////////
// Camera Fly     //
////////////////////
class Camera {
private:
    GLint vp_loc;
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 world_up;
    float yaw;
    float pitch;
    float speed;
    float sensitivity;

public:
    Camera (Shaders& shaders) {
        vp_loc = glGetUniformLocation (shaders.program, "vp");
        reset();
        speed = 2.0f;
        sensitivity = 0.2f;
    }

    void reset() {
        position = glm::vec3(0.0f, 150.0f, 150.0f); // Coordinate alte per vedere il DEM
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
        update_matrices();
    }

    void process_mouse_movement(float xoffset, float yoffset) {
        xoffset *= sensitivity;
        yoffset *= sensitivity;
        yaw += xoffset;
        pitch -= yoffset; 
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
        update_camera_vectors();
    }

    void update_matrices() {
        glm::mat4 view = glm::lookAt(position, position + front, up);
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 1000.0f);
        glm::mat4 vp = projection * view;
        glUniformMatrix4fv(vp_loc, 1, GL_FALSE, &vp[0][0]);
    }

private:
    void update_camera_vectors() {
        glm::vec3 new_front;
        new_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        new_front.y = sin(glm::radians(pitch));
        new_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(new_front);
        right = glm::normalize(glm::cross(front, world_up));
        up    = glm::normalize(glm::cross(right, front));
        update_matrices();
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
    Scene () { 
        send_arrays_2a3f(); 
        generate(4, 0.05f, 10.0f, 12345, false); 
    }
    ~Scene () { clean (); }

    void generate(int octaves, float frequency, float amplitude, unsigned int seed, bool use_dem ) {
        points.clear();
        indices.clear();
        
        Mesh mesh (50, 50, octaves, frequency, amplitude, seed, use_dem); 
        mesh.pack4gpu (points, indices);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }

    void clean () {
        glDeleteVertexArrays (1, &vao);
        glDeleteBuffers (1, &vbo);
        glDeleteBuffers (1, &ebo);
    }

    void draw (bool wireframe) {
        if (wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glDrawElements(GL_TRIANGLES, indices.size (), GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

private:
    void send_arrays_2a3f () {
        glGenBuffers (1, &vbo);
        glBindBuffer (GL_ARRAY_BUFFER, vbo);
        glGenVertexArrays (1, &vao);
        glBindVertexArray (vao);

        // 0: Posizione (X,Y,Z)
        glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray (0);

        // 1: Normali (NX,NY,NZ)
        glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray (1);

        // 2: UV Coordinates (U,V) - Ora ci sono!
        glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray (2);

        glGenBuffers(1, &ebo); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }
};

////////////////////
// SFML Callbacks //
////////////////////
void handle (const sf::Event::KeyPressed& key, Shaders& shaders, Camera& camera) {
    if (ImGui::GetIO().WantCaptureKeyboard) return;
    if (key.scancode == sf::Keyboard::Scancode::R) {
        camera.reset();
    }
    if (key.scancode == sf::Keyboard::Scancode::Space) {
        shaders.reload ("04_vertex.vert", "04_fragment.frag");
        shaders.use ();
    }
}

void handle (const sf::Event::MouseMoved* mouse, Camera& camera) {
    float x = mouse->position.x;
    float y = mouse->position.y;
    static float prev_x = x;
    static float prev_y = y;
    float dx = x - prev_x; 
    float dy = y - prev_y; 
    prev_x = x;
    prev_y = y;

    if (ImGui::GetIO().WantCaptureMouse) return;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
        camera.process_mouse_movement(dx, dy);
    }
}

//////////
// Main //
//////////
int main() {
    unsigned int window_width = 1200;
    unsigned int window_height = 800;

    sf::ContextSettings settings;
    settings.depthBits = 32;
    settings.stencilBits = 8;
    settings.antiAliasingLevel = 4;
    settings.attributeFlags = sf::ContextSettings::Attribute::Core;
    settings.majorVersion = 4;
    settings.minorVersion = 1;

    sf::Window window(
        sf::VideoMode({window_width, window_height}),
        "Generatore Procedurale - Tappa 08 (Texture Avanzate)",
        sf::Style::Default,
        sf::State::Windowed,
        settings
    );
    window.setVerticalSyncEnabled (true);

    if (!window.setActive (true)) return 1;

    int version = gladLoadGL (sf::Context::getFunction);
    if (!version) return 1;

    if (!ImGui::SFML::Init(window, {(float) window_width, (float) window_height})) return 1;
    ImGui_ImplOpenGL3_Init("#version 410 core");

    // 1. CARICAMENTO SHADER TAPPA 08
    Shaders shaders ("04_vertex.vert", "04_fragment.frag");
    shaders.use ();
    
    // 2. LOCAZIONI VARIABILI LUCE
    GLint lightDir_loc = glGetUniformLocation(shaders.program, "lightDir");
    GLint lightColor_loc = glGetUniformLocation(shaders.program, "lightColor");
    GLint ambient_loc = glGetUniformLocation(shaders.program, "ambientStrength");

    // 3. LOCAZIONI VARIABILI BIOMI (Erano queste a mancare all'appello!)
    GLint hWater_loc = glGetUniformLocation(shaders.program, "hWater");
    GLint hSand_loc = glGetUniformLocation(shaders.program, "hSand");
    GLint hGrass_loc = glGetUniformLocation(shaders.program, "hGrass");
    GLint hRock_loc = glGetUniformLocation(shaders.program, "hRock");
    GLint hSnow_loc = glGetUniformLocation(shaders.program, "hSnow");

    // 4. CARICAMENTO TEXTURE
    GLuint texWater = loadTexture("../texture/water.png");
    GLuint texSand  = loadTexture("../texture/sand.png");
    GLuint texGrass = loadTexture("../texture/grass.png");
    GLuint texRock  = loadTexture("../texture/rock.png");
    GLuint texSnow  = loadTexture("../texture/snow.png");

    glUniform1i(glGetUniformLocation(shaders.program, "texWater"), 0);
    glUniform1i(glGetUniformLocation(shaders.program, "texSand"),  1);
    glUniform1i(glGetUniformLocation(shaders.program, "texGrass"), 2);
    glUniform1i(glGetUniformLocation(shaders.program, "texRock"),  3);
    glUniform1i(glGetUniformLocation(shaders.program, "texSnow"),  4);

    Camera camera (shaders);
    Scene scene; 

    // Disabilito il culling per non far sparire il ghiacciaio
    // glEnable (GL_CULL_FACE);
    // glCullFace (GL_BACK);
    glEnable (GL_DEPTH_TEST);

    sf::Clock clock;
    bool running = true;
    while (running) {
        
        while (const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>()) running = false;
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                window_width = resized->size.x;
                window_height = resized->size.y;
                glViewport (0, 0, window_width, window_height);
            }
            if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed> ()) handle (*key_pressed, shaders, camera);
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

        sf::Time elapsed = clock.restart();
        sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::SFML::Update(mouse_pos, {(float) window_width, (float) window_height}, elapsed);

        // --- INTERFACCIA IMGUI ---
        ImGui::Begin("Pannello di Controllo");
        
        // VARIABILI
        static int octaves = 4;
        static float frequency = 0.05f;
        static float amplitude = 10.0f;
        static unsigned int current_seed = 12345;
        static bool use_dem = false; 
        
        static float hWater = 1.0f;
        static float hSand = 2.0f;
        static float hGrass = 6.0f;
        static float hRock = 12.0f;
        static float hSnow = 20.0f;

        static glm::vec3 sun_dir = glm::vec3(0.5f, 1.0f, 0.3f);
        static glm::vec3 sun_color = glm::vec3(1.0f, 0.95f, 0.8f);
        static float ambient_strength = 0.2f;
        static bool wireframe = false;

        bool changed = false;

        // SEZIONE 1: TERRENO
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "MODALITA' TERRENO");
        if (ImGui::RadioButton("Procedurale", !use_dem)) { use_dem = false; changed = true; }
        ImGui::SameLine();
        if (ImGui::RadioButton("Dati Reali (DEM)", use_dem)) { use_dem = true; changed = true; }
        ImGui::Separator();

        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "PARAMETRI PROCEDURALI");
        if (use_dem) ImGui::BeginDisabled(); 
        if (ImGui::Button("Genera Nuovo (Seed)")) { current_seed = std::rand(); changed = true; }
        ImGui::Text("Seed: %u", current_seed);
        if (ImGui::SliderInt("Ottave", &octaves, 1, 8)) changed = true;
        if (ImGui::SliderFloat("Frequenza", &frequency, 0.01f, 0.2f)) changed = true;
        if (ImGui::SliderFloat("Ampiezza", &amplitude, 1.0f, 30.0f)) changed = true;
        if (use_dem) ImGui::EndDisabled();
        ImGui::Separator();

        // SEZIONE 2: SOGLIE BIOMI
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.8f, 1.0f), "SOGLIE BIOMI (TEXTURE)");
        ImGui::SliderFloat("Livello Acqua", &hWater, -2.0f, 5.0f);
        ImGui::SliderFloat("Livello Sabbia", &hSand, 0.0f, 8.0f);
        ImGui::SliderFloat("Livello Erba", &hGrass, 2.0f, 15.0f);
        ImGui::SliderFloat("Livello Roccia", &hRock, 5.0f, 25.0f);
        ImGui::SliderFloat("Livello Neve", &hSnow, 10.0f, 30.0f);

        ImGui::Separator();
        
        // SEZIONE 3: LUCE
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "ILLUMINAZIONE E VISUALIZZAZIONE");
        ImGui::Checkbox("Modalita' Wireframe", &wireframe);
        ImGui::SliderFloat3("Direzione Sole", &sun_dir.x, -1.0f, 1.0f);
        ImGui::ColorEdit3("Colore Sole", &sun_color.x);
        ImGui::SliderFloat("Luce Ambientale", &ambient_strength, 0.0f, 1.0f);
        
        ImGui::End();

        // RIGENERAZIONE SE NECESSARIO
        if (changed) {
            scene.generate(octaves, frequency, amplitude, current_seed, use_dem);
        }

        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- INVIO DATI ALLA GPU ---
        glUniform3f(lightDir_loc, sun_dir.x, sun_dir.y, sun_dir.z);
        glUniform3f(lightColor_loc, sun_color.x, sun_color.y, sun_color.z);
        glUniform1f(ambient_loc, ambient_strength);

        glUniform1f(hWater_loc, hWater);
        glUniform1f(hSand_loc, hSand);
        glUniform1f(hGrass_loc, hGrass);
        glUniform1f(hRock_loc, hRock);
        glUniform1f(hSnow_loc, hSnow);

        // Attiva i layer delle Texture
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texWater);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, texSand);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, texGrass);
        glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, texRock);
        glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, texSnow);

        scene.draw (wireframe); 

        ImGui::SFML::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        window.display();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::SFML::Shutdown();
    return 0;
}