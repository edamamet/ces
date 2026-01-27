#include "SDL3/SDL_scancode.h"
#include <cmath>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "ces_config.h"

struct App {
    SDL_Window* Window;
    SDL_Renderer* Renderer;
    float Eval;
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    SDL_Log("Initializing SDL...");
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize SDL: %s", SDL_GetError());
    }

    SDL_Log("Setting metadata...");
    if (!SDL_SetAppMetadata("ces", "1.0", "com.edamamet.ces")) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to set metadata: %s", SDL_GetError());
    }

    SDL_Log("Allocating appstate...");
    *appstate = SDL_calloc(1, sizeof(App));
    if (*appstate == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to allocate appstate: %s", SDL_GetError());
    }
    App* app = (App*)*appstate;

    SDL_Log("Creating window and renderer...");
    if (!SDL_CreateWindowAndRenderer("ces", 100, 600, SDL_WINDOW_RESIZABLE|SDL_WINDOW_BORDERLESS, &app->Window, &app->Renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create window/renderer: %s", SDL_GetError());
    }
    SDL_RaiseWindow(app->Window);

    SDL_Log("Welcome to ces!");
    return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppIterate(void *appstate) {
    App* app = (App*)appstate;
    SDL_SetRenderDrawColor(app->Renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    SDL_RenderClear(app->Renderer);

    SDL_SetRenderDrawColor(app->Renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    int windowWidth, windowHeight;
    SDL_GetWindowSize(app->Window, &windowWidth, &windowHeight);
    float y = std::lerp((float)windowHeight, 0, app->Eval);
    SDL_FRect white { 0,y,(float)windowWidth,(float)windowHeight };
    SDL_RenderFillRect(app->Renderer, &white);

    SDL_RenderPresent(app->Renderer);
    return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    App* app = (App*)appstate;
    switch (event->type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: 
        {
            return SDL_APP_SUCCESS;
        } break;
        case SDL_EVENT_KEY_DOWN:
        {
            switch (event->key.scancode) {
                case SDL_SCANCODE_1:
                    app->Eval = 0;
                    break;
                case SDL_SCANCODE_2:
                    app->Eval = 1.0f/9;
                    break;
                case SDL_SCANCODE_3:
                    app->Eval = 2.0f/9;
                    break;
                case SDL_SCANCODE_4:
                    app->Eval = 3.0f/9;
                    break;
                case SDL_SCANCODE_5:
                    app->Eval = 4.0f/9;
                    break;
                case SDL_SCANCODE_6:
                    app->Eval = 5.0f/9;
                    break;
                case SDL_SCANCODE_7:
                    app->Eval = 6.0f/9;
                    break;
                case SDL_SCANCODE_8:
                    app->Eval = 7.0f/9;
                    break;
                case SDL_SCANCODE_9:
                    app->Eval = 8.0f/9;
                    break;
                case SDL_SCANCODE_0:
                    app->Eval = 1;
                    break;
                case SDL_SCANCODE_EQUALS:
                    app->Eval = 0.5f;
                    break;
                default: break;
            }
        }

    }
    return SDL_APP_CONTINUE;
}
void SDL_AppQuit(void *appstate, SDL_AppResult result) {}

