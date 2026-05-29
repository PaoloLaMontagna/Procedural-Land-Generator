#define GLAD_GL_IMPLEMENTATION // Necessary for the header-only version.
#include "glad/gl.h" // See file to check used options and generator URL

#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <SFML/Window.hpp>
#include <iostream>


void opengl_stuff ();


int main()
{
    unsigned int window_width = 800;
    unsigned int window_height = 800;

    /////////////////////////////
    // Window and OpenGL setup //
    /////////////////////////////

    // Options for OpenGL context, to be kept in sync with GLAD options!
    sf::ContextSettings settings;
    settings.depthBits = 32;
    settings.stencilBits = 8;
    settings.antiAliasingLevel = 4;
    settings.attributeFlags = sf::ContextSettings::Attribute::Core;
    settings.majorVersion = 4;
    settings.minorVersion = 1;

    // Create the window with chosen options
    sf::Window window(
        sf::VideoMode({window_width, window_height}),
        "SFML + OpenGL",
        sf::Style::Default,
        sf::State::Windowed,
        settings
    );
    window.setVerticalSyncEnabled (true);

    // Activate the window's OpenGL context
    if (!window.setActive (true)) {
        std::cerr << "Failure: error during SFML OpenGL Activation." << std::endl;
        return 1;
    }

    // Check what we have received back
    sf::ContextSettings gotten = window.getSettings();
    std::cout << "depth bits: " << gotten.depthBits << std::endl;
    std::cout << "stencil bits: " << gotten.stencilBits << std::endl;
    std::cout << "antialiasing level: " << gotten.antiAliasingLevel << std::endl;
    std::cout << "SFML GL version: " << gotten.majorVersion << "." << gotten.minorVersion << std::endl;

    int version = gladLoadGL (sf::Context::getFunction);
    if (!version) {
        std::cerr << "Failure: error during glad loading." << std::endl;
        return 1;
    }
    std::cout << "GLAD GL version: "<<GLAD_VERSION_MAJOR(version)<<"."<< GLAD_VERSION_MINOR(version)<<std::endl;

    if (!ImGui::SFML::Init(window, {(float) window_width, (float) window_height})) {
        std::cerr << "Failure: could not init ImGui::SFML." << std::endl;
        return 1;
    }
    ImGui_ImplOpenGL3_Init("#version 410 core");

    ///////////////
    // Main Loop //
    ///////////////

    opengl_stuff ();

    sf::Clock clock;
    bool running = true;
    while (running) {
        // handle events
        while (const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>())
                running = false;
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                window_width = resized->size.x;
                window_height = resized->size.y;
                glViewport (0, 0, window_width, window_height);
            }
        }

        sf::Time elapsed = clock.restart();
        sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::SFML::Update(mouse_pos, {(float) window_width, (float) window_height}, elapsed);
        ImGui::ShowDemoWindow();
        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        // clear the buffers
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui::SFML::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glDrawArrays (GL_TRIANGLES, 0, 3);

        window.display();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::SFML::Shutdown();
    return 0;
}


void opengl_stuff ()
{
    float points[] = {
        0.0f,  0.5f,  0.0f,  // x,y,z of first point
        0.5f, -0.5f,  0.0f,  // x,y,z of second point
       -0.5f, -0.5f,  0.0f   // x,y,z of third point
    };

    GLuint vbo;
    glGenBuffers (1, &vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glBufferData (GL_ARRAY_BUFFER, sizeof (points), points, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays (1, &vao);
    glBindVertexArray (vao);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray (0);

    const char* vertex_source =
        "#version 410 core\n"
        "layout(location = 0) in vec3 vp;"
        "void main() {"
        "  gl_Position = vec4 (vp, 1.0);"
        "}";
    const char* fragment_source =
        "#version 410 core\n"
        "out vec4 frag_colour;"
        "void main() {"
        "  frag_colour = vec4 (0.5, 0.0, 0.5, 1.0);"
        "}";

    GLuint vertex = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource (vertex, 1, &vertex_source, NULL);
    glCompileShader (vertex);
    GLuint fragment = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource (fragment, 1, &fragment_source, NULL);
    glCompileShader (fragment);
    GLuint shader_program = glCreateProgram();
    glAttachShader (shader_program, fragment);
    glAttachShader (shader_program, vertex);
    glLinkProgram (shader_program);

    glDeleteShader (vertex);
    glDeleteShader (fragment);

    glUseProgram (shader_program);
    glBindVertexArray (0);
}
