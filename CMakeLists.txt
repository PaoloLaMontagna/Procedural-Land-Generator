cmake_minimum_required(VERSION 3.5...4.0)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum policy version for all subprojects")

# Nome del progetto globale
project(ProceduralTerrainGenerator LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Abilita la generazione del file compile_commands.json per sistemare i "red squiggles" di VS Code
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

############################
### Library dependencies ###
############################

# 1. Scaricamento SFML 3.0.2
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

# 2. Scaricamento Dear ImGui v1.91.9
FetchContent_Declare(ImGui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG v1.91.9
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(ImGui)
FetchContent_GetProperties(ImGui SOURCE_DIR IMGUI_DIR)

# 3. Scaricamento ImGui-SFML v3.0
set(IMGUI_SFML_FIND_SFML OFF)
FetchContent_Declare(ImGui-SFML
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml
    GIT_TAG v3.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(ImGui-SFML)
set(IMGUI_DIR ${imgui_SOURCE_DIR})

# 4. Scaricamento GLM 1.0.3 (Necessaria per vettori, matrici e telecamera)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(glm)

# Configurazione opzioni compilatore comuni
add_library(common INTERFACE)
target_compile_features(common INTERFACE cxx_std_20)
target_compile_options(common INTERFACE
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wall -DGL_SILENCE_DEPRECATION>
  $<$<CXX_COMPILER_ID:MSVC>: /W4>
)

# Rintraccia OpenGL sul sistema di sviluppo
find_package(OpenGL REQUIRED)


###############################
### Sources and executables ###
###############################

# ==========================================
# --- TAPPA 01: Setup e Griglia Piatta ---
# ==========================================
set(T1 "Tappa01")
add_executable(${T1} 
    Cartella-Tappa01/main.cc 
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_link_libraries(${T1} PRIVATE 
    SFML::Window 
    ImGui-SFML::ImGui-SFML 
    OpenGL::GL 
    glm::glm-header-only
    common
)

target_include_directories(${T1} PRIVATE 
    ${imgui_SOURCE_DIR}  
    ${imgui_SOURCE_DIR}/backends
)


