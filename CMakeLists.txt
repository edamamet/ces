cmake_minimum_required(VERSION 4.0)
project(ces)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(deps)
add_executable(${PROJECT_NAME} src/unity_build.cpp)
target_link_libraries(${PROJECT_NAME} SDL3::SDL3)
