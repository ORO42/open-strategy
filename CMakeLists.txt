cmake_minimum_required(VERSION 3.10)

project(MyGame)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Automatically find all .cpp files in the src directory
file(GLOB SOURCES "src/*.cpp")

# Add Raylib submodule directory
add_subdirectory(raylib)

# Add executable with all source files
add_executable(MyGame ${SOURCES})

# Link Raylib to your project
target_link_libraries(MyGame raylib)
