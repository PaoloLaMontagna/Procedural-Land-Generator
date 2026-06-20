#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"

#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <SFML/Window.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

#include <iostream>
#include <optional>

// I nostri file
#include "include/hotshaders.hh"
#include "00_mesh.hh"

////////////////////
// Camera Orbit   //
////////////////////
class Camera {
private:
    GLint vp_loc;
    float phi_deg = 180.0f;
    float theta_deg = 90.0f; 
    float fd = 80.0f; 
    float od = 80.0f; 
    
public:
    float target_x = -25.0f;
    float target_y = 0.0f;
    float target_z = -25.0f;
    Camera (Shaders& shaders) {
        vp_loc = glGetUniformLocation (shaders.program, "vp");
        update ();
    }

    void drag (float dx, float dy) {
        phi_deg += dx * 0.2f;
        theta_deg += dy * 0.2f;
        if (theta_deg > 90.0f) theta_deg = 90.0f;
        if (theta_deg < -90.0f) theta_deg = -90.0f;
        update ();
    }

    void zoom (float dy) { 
        fd += dy * (fd / 100.0f); 
        if (fd < 0.1f) fd = 0.1f; 
        update (); 
    }

    void dolly (float dy) { 
        od -= dy * (od / 100.0f); 
        if (od < 0.5f) od = 0.5f; 
        update (); 
    }
    // Funzioni per il movimento della telecamera
    void moveForward (float distance) {
        target_z += distance; 
        update (); 
    }
    void moveBackward (float distance) {
        target_z -= distance; 
        update (); 
    }
    void moveLeft (float distance) {
        target_x -= distance; 
        update (); 
    }
    void moveRight (float distance) {
        target_x += distance; 
        update (); 
    }
    void moveup (float distance) {
        target_y += distance; 
        update (); 
    }
    void movedown (float distance) {
        target_y -= distance; 
        update (); 
    }
    void resetview () {
        phi_deg = 180.0f;
        theta_deg = 90.0f;
        fd = 80.0f;
        od = 80.0f;
        target_x = -25.0f;
        target_y = 0.0f;
        target_z = -25.0f;
        update ();
    }

private:
    void update () {    
        float ncp = od - 1.0f; 
        if (ncp < 0.1f) ncp = 0.1f;
        float fcp = od + 1.0f; 

        float ps = glm::sin (glm::radians (phi_deg));
        float pc = glm::cos (glm::radians (phi_deg));
        glm::mat4 ry = glm::mat4(
             pc, 0.0f, -ps, 0.0f, 
            0.0f, 1.0f, 0.0f, 0.0f, 
             ps, 0.0f,  pc, 0.0f, 
            0.0f, 0.0f, 0.0f, 1.0f
        );

        float ts = glm::sin (glm::radians (theta_deg));
        float tc = glm::cos (glm::radians (theta_deg));
        glm::mat4 rx = glm::mat4(
            1.0f, 0.0f, 0.0f, 0.0f, 
            0.0f,  tc, ts,  0.0f, 
            0.0f, -ts, tc,  0.0f, 
            0.0f, 0.0f, 0.0f, 1.0f
        );

        glm::mat4 tz = glm::mat4(
            1.0f, 0.0f, 0.0f, 0.0f, 
            0.0f, 1.0f, 0.0f, 0.0f, 
            0.0f, 0.0f, 1.0f, 0.0f, 
            0.0f, 0.0f, -od, 1.0f  
        );

        // Traslazione per inquadrare il centro della griglia 50x50
        glm::mat4 t_center = glm::mat4(1.0f);
        t_center[3][0] = target_x; 
        t_center[3][1] = target_y; 
        t_center[3][2] = target_z;

        float a = (fcp + ncp) / (ncp - fcp);       
        float b = 2.0f * fcp * ncp / (ncp - fcp);   

        glm::mat4 pr = glm::mat4(
             fd,  0.0f, 0.0f,  0.0f,    
            0.0f,  fd, 0.0f,  0.0f,    
            0.0f, 0.0f,   a, -1.0f,    
            0.0f, 0.0f,   b,  0.0f     
        );

        glm::mat4 vp = pr * tz * rx * ry * t_center;
        glUniformMatrix4fv (vp_loc, 1, GL_FALSE, &vp[0][0]);
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
    Scene () { load (); }
    ~Scene () { clean (); }

    void load () {
        Mesh mesh (50, 50); 
        mesh.pack4gpu (points, indices);
        send_arrays_2a3f ();
    }

    void clean () {
        glDeleteVertexArrays (1, &vao);
        glDeleteBuffers (1, &vbo);
        glDeleteBuffers (1, &ebo);
    }

    void draw () {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, indices.size (), GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

private:
    void send_arrays_2a3f () {
        glGenBuffers (1, &vbo);
        glBindBuffer (GL_ARRAY_BUFFER, vbo);
        glBufferData (GL_ARRAY_BUFFER, points.size () * sizeof (float), points.data (), GL_STATIC_DRAW);

        glGenVertexArrays (1, &vao);
        glBindVertexArray (vao);

        glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray (0);

        glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray (1);

        glGenBuffers(1, &ebo); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size () * sizeof (unsigned int), indices.data (), GL_STATIC_DRAW);
    }
};

////////////////////
// SFML Callbacks //
////////////////////
// WASD MOVEMENT  //
////////////////////
void handleCameraMovement (const sf::Event::KeyPressed& key, Camera& camera) {
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    if (key.scancode == sf::Keyboard::Scancode::W) {
        camera.moveForward (1.0f);
    } else if (key.scancode == sf::Keyboard::Scancode::S) {
        camera.moveBackward (1.0f);
    } else if (key.scancode == sf::Keyboard::Scancode::A) {
        camera.moveLeft (1.0f);
    } else if (key.scancode == sf::Keyboard::Scancode::D) {
        camera.moveRight (1.0f);
    } else if (key.scancode == sf::Keyboard::Scancode::Q) {
        camera.moveup (1.0f);
    } else if (key.scancode == sf::Keyboard::Scancode::E) {
        camera.movedown (1.0f);
    } else if (key.scancode == sf::Keyboard::Scancode::R) {
        camera.resetview ();
    }
}
void handle (const sf::Event::KeyPressed& key, Shaders& shaders) {
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    if (key.scancode == sf::Keyboard::Scancode::Space) {
        shaders.reload ("00_vertex.vert", "00_fragment.frag");
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

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) camera.drag (dx, dy);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) camera.zoom (dy);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LAlt)) camera.dolly (dy);
}

//////////
// Main //
//////////
int main() {
    unsigned int window_width = 1000;
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
        "Generatore Procedurale - Tappa 02",
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

    Shaders shaders ("00_vertex.vert", "00_fragment.frag");
    shaders.use ();
    Camera camera (shaders);
    Scene scene; 

    glEnable (GL_CULL_FACE);
    glCullFace (GL_BACK);
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
            if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed> ()){ handle (*key_pressed, shaders); handleCameraMovement (*key_pressed, camera);};
            if (const auto* mouse = event->getIf<sf::Event::MouseMoved> ()) handle (mouse, camera);
        }

        sf::Time elapsed = clock.restart();
        sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::SFML::Update(mouse_pos, {(float) window_width, (float) window_height}, elapsed);

        ImGui::Begin("Parametri Generazione");
        ImGui::Text("Tappa 02: WASD per muoversi.\nQ/E per salire/abbassare, R per resettare la vista.");
        ImGui::Text("Trascina il Tasto Sinistro per ruotare.");
        ImGui::Text("Tieni premuto CTRL + muovi il mouse per lo Zoom.");
        ImGui::End();

        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene.draw (); 

        ImGui::SFML::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.display();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::SFML::Shutdown();
    return 0;
}