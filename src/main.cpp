#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cmath>

#include "ces_config.h"
#include "typedefs.h"
#include "lemath.h"

struct Eval {
    float Eval;
    float Eval01;
};

struct App {
    SDL_Window *Window;
    SDL_Renderer *Renderer;

    TTF_TextEngine *TextEngine;
    TTF_Font *RegularFont;
    TTF_Font *ItalicFont;

    double Delta;
    u64 PreviousTicks;

    Eval TrueEval;
    Eval AnimatedEval;
    float Speed;

    bool DisplayFPS;
};

void CalculateDeltaTime(App *app) {
    u64 currentTicks = SDL_GetTicksNS();
    app->Delta = (currentTicks - app->PreviousTicks) / 1e9;
    app->PreviousTicks = currentTicks;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    SDL_Log("Initializing SDL...");
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Log("Setting metadata...");
    if (!SDL_SetAppMetadata("ces", "1.0", "com.edamamet.ces")) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to set metadata: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Log("Allocating appstate...");
    *appstate = SDL_calloc(1, sizeof(App));
    if (*appstate == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to allocate appstate: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    App* app = (App*)*appstate;

    SDL_Log("Creating window and renderer...");
    if (!SDL_CreateWindowAndRenderer("ces", 100, 600, SDL_WINDOW_RESIZABLE|SDL_WINDOW_BORDERLESS, &app->Window, &app->Renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_RaiseWindow(app->Window);

    SDL_Log("Initializing fonts");
    if (!TTF_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize SDL_TTF: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    app->TextEngine = TTF_CreateRendererTextEngine(app->Renderer);
    if (app->TextEngine == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize the text engine: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    app->RegularFont = TTF_OpenFont(FONT_REGULAR_PATH, FONT_SIZE);
    if (app->RegularFont == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to open font: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    app->ItalicFont = TTF_OpenFont(FONT_ITALIC_PATH, FONT_SIZE);
    if (app->ItalicFont == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to open font: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Log("Initializing game data");
    app->TrueEval.Eval01 = 0.5f;
    app->AnimatedEval.Eval01 = 0.5f;
    
    SDL_Log("Welcome to ces!");
    return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppIterate(void *appstate) {
    App* app = (App*)appstate;

    CalculateDeltaTime(app);

    app->AnimatedEval.Eval01 = MoveTowards(app->AnimatedEval.Eval01, app->TrueEval.Eval01, app->Speed * app->Delta);

    SDL_SetRenderDrawColor(app->Renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    SDL_RenderClear(app->Renderer);

    SDL_SetRenderDrawColor(app->Renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    int windowWidth, windowHeight;
    SDL_GetWindowSize(app->Window, &windowWidth, &windowHeight);
    float y = std::lerp((float)windowHeight, 0, app->AnimatedEval.Eval01);
    SDL_FRect white { 0,y,(float)windowWidth,(float)windowHeight };
    SDL_RenderFillRect(app->Renderer, &white);

    if (app->TrueEval.Eval01 >= 0.5f) {
        int windowWidth, windowHeight;
        SDL_GetWindowSize(app->Window, &windowWidth, &windowHeight);

        char rawText[6]; // +xx.x\0
        SDL_snprintf(rawText, sizeof(rawText), "+%.1f", app->TrueEval.Eval);
        TTF_Text *text = TTF_CreateText(app->TextEngine, app->RegularFont, rawText , 0);
        TTF_SetTextColor(text, BLACK.r, BLACK.g, BLACK.g, BLACK.a);
        int textWidth;
        TTF_MeasureString(app->RegularFont, rawText, 0, 0, &textWidth, nullptr);
        float x = ((float)windowWidth / 2)-((float)textWidth / 2);
        float y = windowHeight - FONT_SIZE - 10;
        if (!TTF_DrawRendererText(text, x, y)) {
            SDL_Log("%s", SDL_GetError());
        }
        TTF_DestroyText(text);
    } else {
        int windowWidth, windowHeight;
        SDL_GetWindowSize(app->Window, &windowWidth, &windowHeight);

        char rawText[6]; // -xx.x\0
        SDL_snprintf(rawText, sizeof(rawText), "%.1f", app->TrueEval.Eval);
        TTF_Text *text = TTF_CreateText(app->TextEngine, app->RegularFont, rawText , 0);
        TTF_SetTextColor(text, WHITE.r, WHITE.g, WHITE.g, WHITE.a);
        int textWidth;
        TTF_MeasureString(app->RegularFont, rawText, 0, 0, &textWidth, nullptr);
        float x = ((float)windowWidth / 2)-((float)textWidth / 2);
        float y = 10;
        if (!TTF_DrawRendererText(text, x, y)) {
            SDL_Log("%s", SDL_GetError());
        }
        TTF_DestroyText(text);
    }

    if (app->DisplayFPS) {
        char rawText[10]; // xxxxx fps
        SDL_snprintf(rawText, sizeof(rawText), "%.0f fps", 1.0 / app->Delta);
        TTF_Text *text = TTF_CreateText(app->TextEngine, app->ItalicFont, rawText, 0);
        int textWidth;
        TTF_MeasureString(app->ItalicFont, rawText, 0, 0, &textWidth, nullptr);

        float padding = 10.0f;
        float w = textWidth + padding;
        float h = FONT_SIZE + padding;

        int windowWidth, windowHeight;
        SDL_GetWindowSize(app->Window, &windowWidth, &windowHeight);

        float centerX = (float)windowWidth/2;
        float centerY = (float)windowHeight/2;

        SDL_FRect debugBg {centerX-(w/2), centerY-(h/2), w, h};
        SDL_SetRenderDrawColor(app->Renderer, DEBUG_BG.r, DEBUG_BG.g, DEBUG_BG.b, DEBUG_BG.a);
        SDL_RenderFillRect(app->Renderer, &debugBg);
        TTF_SetTextColor(text, DEBUG_FG.r, DEBUG_FG.g, DEBUG_FG.b, DEBUG_FG.a);
        TTF_DrawRendererText(text, centerX - (float)textWidth/2, centerY - (float)FONT_SIZE / 2);

        TTF_DestroyText(text);
    }

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
            float previousEval = app->TrueEval.Eval01;
            switch (event->key.scancode) {
                case SDL_SCANCODE_1:
                    app->TrueEval.Eval01 = 0;
                    break;
                case SDL_SCANCODE_2:
                    app->TrueEval.Eval01 = 1.0f/9;
                    break;
                case SDL_SCANCODE_3:
                    app->TrueEval.Eval01 = 2.0f/9;
                    break;
                case SDL_SCANCODE_4:
                    app->TrueEval.Eval01 = 3.0f/9;
                    break;
                case SDL_SCANCODE_5:
                    app->TrueEval.Eval01 = 4.0f/9;
                    break;
                case SDL_SCANCODE_6:
                    app->TrueEval.Eval01 = 5.0f/9;
                    break;
                case SDL_SCANCODE_7:
                    app->TrueEval.Eval01 = 6.0f/9;
                    break;
                case SDL_SCANCODE_8:
                    app->TrueEval.Eval01 = 7.0f/9;
                    break;
                case SDL_SCANCODE_9:
                    app->TrueEval.Eval01 = 8.0f/9;
                    break;
                case SDL_SCANCODE_0:
                    app->TrueEval.Eval01 = 1;
                    break;
                case SDL_SCANCODE_EQUALS:
                    app->TrueEval.Eval01 = 0.5f;
                    break;
                case SDL_SCANCODE_F: 
                    app->DisplayFPS = !app->DisplayFPS;
                    break;
                default: break;
            }
            app->Speed = SDL_fabsf(app->TrueEval.Eval01 - app->AnimatedEval.Eval01);
            app->TrueEval.Eval = (app->TrueEval.Eval01 - 0.5f) * 2 * MAX_SCORE;
        }

    }
    return SDL_APP_CONTINUE;
}
void SDL_AppQuit(void *appstate, SDL_AppResult result) {}

