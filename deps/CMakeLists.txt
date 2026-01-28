project(deps)
include(FetchContent)

add_library(${PROJECT_NAME} INTERFACE)

set(SDLTTF_VENDORED ON)
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/deps)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(
    SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-3.2.x
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3_ttf)

target_link_libraries(${PROJECT_NAME} INTERFACE 
    SDL3
    SDL3_ttf
)
