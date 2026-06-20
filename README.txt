 Compilation:
```
cmake -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
```
`Release`  can be changed to `Debug` if necessary.



# ==========================================
# --- TAPPA 02: Telecamera Fly (WASD)   ---
# ==========================================
set(T2 "Tappa02")
add_executable(${T2} 
    Cartella-Tappa02/main.cc 
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_link_libraries(${T2} PRIVATE 
    SFML::Window 
    ImGui-SFML::ImGui-SFML 
    OpenGL::GL 
    glm::glm-header-only
    common
)

target_include_directories(${T2} PRIVATE 
    ${imgui_SOURCE_DIR}  
    ${imgui_SOURCE_DIR}/backends
)

# Nota metodologica per il futuro: Quando sarai pronto a creare la Tappa 03, 
# ti basterà fare un copia-incolla del blocco qui sopra, cambiando T2 in T3 
# e i puntamenti a Cartella-Tappa03.