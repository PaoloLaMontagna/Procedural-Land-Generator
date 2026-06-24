#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"

#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <SFML/Window.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <iostream>
#include <optional>
#include <cstdlib> // Per std::rand()

#include "include/hotshaders.hh"
#include "00_mesh.hh"
#include "dem.hh"

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
        
        position = glm::vec3(25.0f, 150.0f, 150.0f); 
        world_up = glm::vec3(0.0f, 1.0f, 0.0f);
        yaw = -90.0f;   
        pitch = -25.0f; 
        
        speed = 2.0f;
        sensitivity = 0.2f;

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
    void reset() {
        position = glm::vec3(25.0f, 15.0f, 50.0f); // Coordinate iniziali
        yaw = -90.0f;   // Guarda dritto in avanti
        pitch = -25.0f; // Guarda leggermente in basso
        update_camera_vectors(); // Ricalcola i vettori di direzione e aggiorna le matrici
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
        generate(4, 0.05f, 10.0f, 12345,false); // Generazione di default
    }
    ~Scene () { clean (); }

    void generate(int octaves, float frequency, float amplitude, unsigned int seed,bool use_dem ) {
        points.clear();
        indices.clear();
        
        Mesh mesh (50, 50, octaves, frequency, amplitude, seed,use_dem); 
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
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Reset sicurezza
    }

private:
    void send_arrays_2a3f () {
        glGenBuffers (1, &vbo);
        glBindBuffer (GL_ARRAY_BUFFER, vbo);

        glGenVertexArrays (1, &vao);
        glBindVertexArray (vao);

        // Attributo 0: Posizione (X, Y, Z)
        glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray (0);

        // Attributo 1: Normali (NX, NY, NZ)
        glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray (1);

        glGenBuffers(1, &ebo); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }
};

////////////////////
// SFML Callbacks //
////////////////////
void handle (const sf::Event::KeyPressed& key, Shaders& shaders, Camera& camera) {
    if (ImGui::GetIO().WantCaptureKeyboard) return;
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        if (key.scancode == sf::Keyboard::Scancode::R) {
            camera.reset();
        }
        if (key.scancode == sf::Keyboard::Scancode::Space) {
            shaders.reload ("02_vertex.vert", "02_fragment.frag");
            shaders.use ();
        }
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
        "Generatore Procedurale - Tappa 06",
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

    // CARICAMENTO SHADER TAPPA 06
    Shaders shaders ("02_vertex.vert", "02_fragment.frag");
    shaders.use ();
    
    // LOCAZIONI VARIABILI LUCE
    GLint lightDir_loc = glGetUniformLocation(shaders.program, "lightDir");
    GLint lightColor_loc = glGetUniformLocation(shaders.program, "lightColor");
    GLint objectColor_loc = glGetUniformLocation(shaders.program, "objectColor");
    GLint ambient_loc = glGetUniformLocation(shaders.program, "ambientStrength");

    Camera camera (shaders);
    Scene scene; 

    //glEnable (GL_CULL_FACE);
    //glCullFace (GL_BACK);
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
            if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed> ()) handle (*key_pressed, shaders,camera);
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
        
        static int octaves = 4;
        static float frequency = 0.05f;
        static float amplitude = 10.0f;
        static unsigned int current_seed = 12345;
        static bool use_dem = false; 
        
        bool changed = false;

        // --- SELEZIONE MODALITA' ---
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "MODALITA' TERRENO");
        if (ImGui::RadioButton("Procedurale (Perlin)", !use_dem)) { use_dem = false; changed = true; }
        ImGui::SameLine();
        if (ImGui::RadioButton("Dati Reali (DEM Aletsch)", use_dem)) { use_dem = true; changed = true; }
        ImGui::Separator();

        // --- CONTROLLI PROCEDURALI ---
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "PARAMETRI PROCEDURALI");
        
        // Se usiamo il DEM, disabilitiamo visivamente i controlli del Perlin
        if (use_dem) ImGui::BeginDisabled(); 
        
        if (ImGui::Button("Reset Configurazione")) {
            octaves = 4; frequency = 0.05f; amplitude = 10.0f; current_seed = 12345;
            changed = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Genera Nuovo (Seed)")) { current_seed = std::rand(); changed = true; }
        
        ImGui::Text("Seed: %u", current_seed);
        if (ImGui::SliderInt("Ottave", &octaves, 1, 8)) changed = true;
        if (ImGui::SliderFloat("Frequenza", &frequency, 0.01f, 0.2f)) changed = true;
        if (ImGui::SliderFloat("Ampiezza", &amplitude, 1.0f, 30.0f)) changed = true;
        
        if (use_dem) ImGui::EndDisabled();
        static bool wireframe = false;
        ImGui::Checkbox("Modalita' Wireframe", &wireframe);

        ImGui::Separator();
        
        //Illuminazione
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "ILLUMINAZIONE (SOLE)");
        static glm::vec3 sun_dir = glm::vec3(0.5f, 1.0f, 0.3f);
        static glm::vec3 sun_color = glm::vec3(1.0f, 0.95f, 0.8f);
        static glm::vec3 terrain_color = glm::vec3(0.3f, 0.6f, 0.3f);
        static float ambient_strength = 0.2f;

        ImGui::SliderFloat3("Direzione Sole", &sun_dir.x, -1.0f, 1.0f);
        ImGui::ColorEdit3("Colore Sole", &sun_color.x);
        ImGui::ColorEdit3("Colore Terreno", &terrain_color.x);
        ImGui::SliderFloat("Luce Ambientale", &ambient_strength, 0.0f, 1.0f);
        if (ImGui::Button("Reset luce")) {
            sun_dir = glm::vec3(0.5f, 1.0f, 0.3f);
            sun_color = glm::vec3(1.0f, 0.95f, 0.8f);
            terrain_color = glm::vec3(0.3f, 0.6f, 0.3f);
            ambient_strength = 0.2f;
        }
        ImGui::End();

        if (changed) {
            scene.generate(octaves, frequency, amplitude, current_seed,use_dem);
        }

        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- INVIA I DATI DELLA LUCE ALLA SCHEDA VIDEO ---
        glUniform3f(lightDir_loc, sun_dir.x, sun_dir.y, sun_dir.z);
        glUniform3f(lightColor_loc, sun_color.x, sun_color.y, sun_color.z);
        glUniform3f(objectColor_loc, terrain_color.x, terrain_color.y, terrain_color.z);
        glUniform1f(ambient_loc, ambient_strength);

        scene.draw (wireframe); 

        ImGui::SFML::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.display();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::SFML::Shutdown();
    return 0;
}