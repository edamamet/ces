project(deps)
include(FetchContent)

add_library(${PROJECT_NAME} INTERFACE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(SDL3)
target_link_libraries(${PROJECT_NAME} INTERFACE SDL3)
