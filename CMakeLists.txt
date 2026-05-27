cmake_minimum_required(VERSION 3.5...4.0)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum policy version for all subprojects")

project(SFMLOpenGLExample LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)


#############################
### Download project data ###
#############################

set(FETCH_CONTENT_NAME "content")
set(DOWNLOAD_FILENAME "data.zip")
FetchContent_Declare(
    ${FETCH_CONTENT_NAME}
    URL https://www.dropbox.com/scl/fi/wfb1k6dudfjbfumajoqs4/data.zip?rlkey=wtq3prdzgwaq5x2z34091gmgc&dl=1
    DOWNLOAD_NAME ${DOWNLOAD_FILENAME}
    DOWNLOAD_NO_EXTRACT TRUE
    SOURCE_DIR ${CMAKE_BINARY_DIR}/
)

FetchContent_MakeAvailable(${FETCH_CONTENT_NAME})

# Manual extraction using the filename variable
execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar -xzf "${CMAKE_BINARY_DIR}/${DOWNLOAD_FILENAME}"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Clean up using variables
# file(REMOVE "${CMAKE_BINARY_DIR}/${DOWNLOAD_FILENAME}")
file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/${FETCH_CONTENT_NAME}-build")
file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/${FETCH_CONTENT_NAME}-tmp")


############################
### Library dependencies ###
############################

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	1.0.3
)
FetchContent_MakeAvailable(glm)

add_library(common INTERFACE)
target_compile_features(common INTERFACE cxx_std_20)
target_compile_options(common INTERFACE
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wall -DGL_SILENCE_DEPRECATION>
  $<$<CXX_COMPILER_ID:MSVC>: /W4>
)

# Find OpenGL
find_package(OpenGL REQUIRED)


###############################
### Sources and executables ###
###############################

# --- TAPPA 01 ---
add_executable(Tappa01 
    Cartella-Tappa01/00_mesh.cc 
)
# Nota: i path ai file .cc includono il nome della cartella, come da regolamento
target_link_libraries(Tappa01 PRIVATE common SFML::Window OpenGL::GL glm::glm-header-only)


# Quando creerai la Tappa 02, ti basterà aggiungere questo:
# add_executable(Tappa02 Cartella-Tappa02/00_mesh.cc)
# target_link_libraries(Tappa02 PRIVATE common SFML::Window OpenGL::GL glm::glm-header-only)