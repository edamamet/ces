#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cfloat>

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

    SDL_Window *SettingsWindow;
    SDL_Renderer *SettingsRenderer;

    SDL_Surface *SettingsBgSurface;
    SDL_Texture *SettingsBgTexture;

    TTF_TextEngine *TextEngine;
    TTF_Font *RegularFont;
    TTF_Font *ItalicFont;

    float Delta;
    u64 PreviousTicks;

    Eval TrueEval;
    Eval AnimatedEval;
    bool IsCheckmate;
    int CheckmateMoves;
    float Speed;

    float TimeSinceLastPress;
    float AdjustmentTimer;
    bool CanAdjust;

    bool IsWhiteModDown;
    bool IsBlackModDown;
    bool IsFastScrollDown;

    bool DisplayFPS;
};

enum Side {
    Black,
    White
};

SDL_HitTestResult SDLCALL WindowHit(SDL_Window *window, const SDL_Point *area, void *data) {
    (void) data;
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    if (area->y <= BORDER_SIZE && area->x <= BORDER_SIZE) {
        return SDL_HITTEST_RESIZE_TOPLEFT;
    }
    if (area->y <= BORDER_SIZE && area->x >= w - BORDER_SIZE) {
        return SDL_HITTEST_RESIZE_TOPRIGHT;
    }
    if (area->y >= h - BORDER_SIZE && area->x <= BORDER_SIZE) {
        return SDL_HITTEST_RESIZE_BOTTOMLEFT;
    }
    if (area->y >= h - BORDER_SIZE && area->x >= w - BORDER_SIZE) {
        return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
    }

    if (area->y <= BORDER_SIZE) {
        return SDL_HITTEST_RESIZE_TOP;
    }
    if (area->y >= h - BORDER_SIZE) {
        return SDL_HITTEST_RESIZE_BOTTOM;
    }
    if (area->x <= BORDER_SIZE) {
        return SDL_HITTEST_RESIZE_LEFT;
    }
    if (area->x >= w - BORDER_SIZE) {
        return SDL_HITTEST_RESIZE_RIGHT;
    }

    return SDL_HITTEST_DRAGGABLE;
}

void CalculateDeltaTime(App *app) {
    u64 currentTicks = SDL_GetTicksNS();
    app->Delta = (float)(currentTicks - app->PreviousTicks) / 1e9f;
    app->PreviousTicks = currentTicks;
}

void SetEval01(App *app, float new01) {
    new01 = SDL_clamp(new01, 0, 1);
    app->TrueEval.Eval01 = new01;
    app->Speed = SDL_fabsf(app->TrueEval.Eval01 - app->AnimatedEval.Eval01);
    app->TrueEval.Eval = (app->TrueEval.Eval01 - 0.5f) * 2 * MAX_SCORE;
}

void SetCheckmate(App *app, Side side, int moves) {
    app->CanAdjust = false;
    app->IsCheckmate = true;
    app->CheckmateMoves = SDL_clamp(moves, 0, 549);
    if (side == Side::White) {
        SetEval01(app, 1);
    } else {
        SetEval01(app, 0);
    }

    app->Speed *= 1.5f;
}

void ResetAdjustmentTime(App *app) {
    app->CanAdjust = true;

    app->TimeSinceLastPress = 0;

    float nextTime = 2 * SDL_randf();
    app->AdjustmentTimer = nextTime;
}

void HandleAdjustment(App *app) {
    if (!app->CanAdjust) return;
    app->TimeSinceLastPress += app->Delta;
    app->AdjustmentTimer -= app->Delta;

    if (app->TimeSinceLastPress >= MAX_ADJUSTMENT_TIME) return;
    if (app->AdjustmentTimer <= 0) {
        float factor = ((MAX_ADJUSTMENT_TIME - app->TimeSinceLastPress) / MAX_ADJUSTMENT_TIME);
        SetEval01(app, app->TrueEval.Eval01 + ((SDL_randf() * 2) - 1) * SDL_powf(factor, 1.5f) * 0.05f);
        float nextTime = SDL_min(app->TimeSinceLastPress * SDL_randf(), 0.75f);
        app->AdjustmentTimer = nextTime;
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    (void) argc;
    (void) argv;

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

    SDL_Log("Querying render driver support...");
    for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
        SDL_Log("  - %s", SDL_GetRenderDriver(i));
    }


    SDL_Log("Creating window and renderer...");
    if (!SDL_CreateWindowAndRenderer("ces", 100, 600, SDL_WINDOW_RESIZABLE|SDL_WINDOW_BORDERLESS|SDL_WINDOW_ALWAYS_ON_TOP, &app->Window, &app->Renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_RaiseWindow(app->Window);

    if (!SDL_SetWindowHitTest(app->Window, WindowHit, app)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to setup custom window handling: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_SetWindowMinimumSize(app->Window, 50, 50)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to set the window\'s minimum size: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
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

    SDL_Log("Loading textures...");
    app->SettingsBgSurface = SDL_LoadPNG(SETTINGS_BG_PATH);
    if (app->SettingsBgSurface == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Log("Initializing game data");
    app->TrueEval.Eval01 = 0.5f;
    app->AnimatedEval.Eval01 = 0.5f;
    app->TimeSinceLastPress = FLT_MAX;
    
    SDL_Log("Welcome to ces!");
    return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppIterate(void *appstate) {
    App* app = (App*)appstate;

    CalculateDeltaTime(app);
    HandleAdjustment(app);

    app->AnimatedEval.Eval01 = MoveTowards(app->AnimatedEval.Eval01, app->TrueEval.Eval01, app->Speed * app->Delta);

    SDL_SetRenderDrawColor(app->Renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    SDL_RenderClear(app->Renderer);

    SDL_SetRenderDrawColor(app->Renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    int windowWidth;
    int windowHeight;
    SDL_GetWindowSize(app->Window, &windowWidth, &windowHeight);
    float barY = Lerp((float)windowHeight, 0, app->AnimatedEval.Eval01);
    SDL_FRect white { 0, barY, (float)windowWidth,(float)windowHeight };
    SDL_RenderFillRect(app->Renderer, &white);

    if (app->TrueEval.Eval01 >= 0.5f) {
        SDL_GetWindowSize(app->Window, &windowWidth, &windowHeight);

        char rawText[6]; // +xx.x\0
        if (app->IsCheckmate) {
            if (app->CheckmateMoves == 0) {
                SDL_snprintf(rawText, sizeof(rawText), "1-0");
            } else {
                SDL_snprintf(rawText, sizeof(rawText), "M%i", app->CheckmateMoves);
            }
        } else {
            SDL_snprintf(rawText, sizeof(rawText), "+%.1f", app->TrueEval.Eval);
        }
        TTF_Text *text = TTF_CreateText(app->TextEngine, app->RegularFont, rawText , 0);
        TTF_SetTextColor(text, BLACK.r, BLACK.g, BLACK.g, BLACK.a);
        int textWidth;
        TTF_MeasureString(app->RegularFont, rawText, 0, 0, &textWidth, nullptr);
        float x = ((float)windowWidth / 2)-((float)textWidth / 2);
        float y = (float)windowHeight - FONT_SIZE - 10;
        if (!TTF_DrawRendererText(text, x, y)) {
            SDL_Log("%s", SDL_GetError());
        }
        TTF_DestroyText(text);
    } else {
        SDL_GetWindowSize(app->Window, &windowWidth, &windowHeight);

        char rawText[6]; // -xx.x\0
        if (app->IsCheckmate) {
            if (app->CheckmateMoves == 0) {
                SDL_snprintf(rawText, sizeof(rawText), "0-1");
            } else {
                SDL_snprintf(rawText, sizeof(rawText), "M%i", app->CheckmateMoves);
            }
        } else {
            SDL_snprintf(rawText, sizeof(rawText), "%.1f", app->TrueEval.Eval);
        }
        TTF_Text *text = TTF_CreateText(app->TextEngine, app->RegularFont, rawText , 0);
        TTF_SetTextColor(text, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
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
        float w = (float)textWidth + padding;
        float h = FONT_SIZE + padding;

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

    if (app->SettingsWindow != nullptr) {
        SDL_RenderClear(app->SettingsRenderer);
        
        SDL_RenderTextureTiled(app->SettingsRenderer, app->SettingsBgTexture, nullptr, 1, nullptr);

        SDL_RenderPresent(app->SettingsRenderer);
    }

    return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    App* app = (App*)appstate;
    switch (event->type) {
        case SDL_EVENT_QUIT: 
            return SDL_APP_SUCCESS;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: 
        {
            SDL_Window* window = SDL_GetWindowFromID(event->window.windowID);
            if (window == app->SettingsWindow) {
                SDL_DestroyWindow(app->SettingsWindow);
                SDL_DestroyRenderer(app->SettingsRenderer);
                app->SettingsWindow = nullptr;
                app->SettingsRenderer = nullptr;
            } else {
                return SDL_APP_SUCCESS;
            }
        } break;
        case SDL_EVENT_KEY_DOWN:
        {
            switch (event->key.scancode) { 
                // mod keys
                case SDL_SCANCODE_LSHIFT:
                    app->IsFastScrollDown = true;
                    break;               
                case SDL_SCANCODE_B:
                    app->IsBlackModDown = true;
                    break;
                case SDL_SCANCODE_W:
                    app->IsWhiteModDown = true;
                    break;

                    // num keys
                case SDL_SCANCODE_1:
                    if (app->IsWhiteModDown) {
                        SetCheckmate(app, Side::White, 1);
                    } else if (app->IsBlackModDown) {
                        SetCheckmate(app, Side::Black, 1);
                    } else {
                        app->IsCheckmate = false;
                        ResetAdjustmentTime(app);
                        SetEval01(app, 0);
                    }
                    break;
                case SDL_SCANCODE_2:
                    if (app->IsWhiteModDown) {
                        SetCheckmate(app, Side::White, 2);
                    } else if (app->IsBlackModDown) {
                        SetCheckmate(app, Side::Black, 2);
                    } else {
                        app->IsCheckmate = false;
                        ResetAdjustmentTime(app);
                        SetEval01(app, 1.0f/9);
                    }
                    break;
                case SDL_SCANCODE_3:
                    if (app->IsWhiteModDown) {
                        SetCheckmate(app, Side::White, 3);
                    } else if (app->IsBlackModDown) {
                        SetCheckmate(app, Side::Black, 3);
                    } else {
                        app->IsCheckmate = false;
                        ResetAdjustmentTime(app);
                        SetEval01(app, 2.0f/9);
                    }
                    break;
                case SDL_SCANCODE_4:
                    if (app->IsWhiteModDown) {
                        SetCheckmate(app, Side::White, 4);
                    } else if (app->IsBlackModDown) {
                        SetCheckmate(app, Side::Black, 4);
                    } else {
                        app->IsCheckmate = false;
                        ResetAdjustmentTime(app);
                        SetEval01(app, 3.0f/9);
                    }
                    break;
                case SDL_SCANCODE_5:
                    if (app->IsWhiteModDown) {
                        SetCheckmate(app, Side::White, 5);
                    } else if (app->IsBlackModDown) {
                        SetCheckmate(app, Side::Black, 5);
                    } else {
                        app->IsCheckmate = false;
                        ResetAdjustmentTime(app);
                        SetEval01(app, 4.0f/9);
                    }
                    break;
                case SDL_SCANCODE_6:
                    if (app->IsWhiteModDown) {
                        SetCheckmate(app, Side::White, 6);
                    } else if (app->IsBlackModDown) {
                        SetCheckmate(app, Side::Black, 6);
                    } else {
                        app->IsCheckmate = false;
                        ResetAdjustmentTime(app);
                        SetEval01(app, 5.0f/9);
                    }
                    break;
                case SDL_SCANCODE_7:
                    if (app->IsWhiteModDown) {
                        SetCheckmate(app, Side::White, 7);
                    } else if (app->IsBlackModDown) {
                        SetCheckmate(app, Side::Black, 7);
                    } else {
                        app->IsCheckmate = false;
                        ResetAdjustmentTime(app);
                        SetEval01(app, 6.0f/9);
                    }
                    break;
                case SDL_SCANCODE_8:
                    if (app->IsWhiteModDown) {
                        SetCheckmate(app, Side::White, 8);
                    } else if (app->IsBlackModDown) {
                        SetCheckmate(app, Side::Black, 8);
                    } else {
                        app->IsCheckmate = false;
                        ResetAdjustmentTime(app);
                        SetEval01(app, 7.0f/9);
                    }
                    break;
                case SDL_SCANCODE_9:
                    if (app->IsWhiteModDown) {
                        SetCheckmate(app, Side::White, 9);
                    } else if (app->IsBlackModDown) {
                        SetCheckmate(app, Side::Black, 9);
                    } else {
                        app->IsCheckmate = false;
                        ResetAdjustmentTime(app);
                        SetEval01(app, 8.0f/9);
                    }
                    break;
                case SDL_SCANCODE_0:
                    if (app->IsWhiteModDown) {
                        SetCheckmate(app, Side::White, 0);
                    } else if (app->IsBlackModDown) {
                        SetCheckmate(app, Side::Black, 0);
                    } else {
                        app->IsCheckmate = false;
                        ResetAdjustmentTime(app);
                        SetEval01(app, 1);
                    }
                    break;
                case SDL_SCANCODE_R:
                case SDL_SCANCODE_EQUALS:
                    app->IsCheckmate = false;
                    app->CanAdjust = false;
                    app->CheckmateMoves = 0;
                    SetEval01(app, 0.5f);
                    break;
                case SDL_SCANCODE_F: 
                    app->DisplayFPS = !app->DisplayFPS;
                    break;
                case SDL_SCANCODE_ESCAPE:
                case SDL_SCANCODE_Q:
                    return SDL_APP_SUCCESS;
                case SDL_SCANCODE_SLASH:
                    {
                        if (app->SettingsWindow != nullptr || app->SettingsRenderer != nullptr) {
                            SDL_Log("Settings window is already active, focusing settings...");
                            SDL_RaiseWindow(app->SettingsWindow);
                            break;
                        }
                        if (!SDL_CreateWindowAndRenderer("", 600, 400, 0, &app->SettingsWindow, &app->SettingsRenderer)) {
                            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create window/renderer: %s", SDL_GetError());
                        }

                        app->SettingsBgTexture = SDL_CreateTextureFromSurface(app->SettingsRenderer, app->SettingsBgSurface);
                        if (app->SettingsBgTexture == nullptr) {
                            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize resources for settings window: %s", SDL_GetError());
                            return SDL_APP_FAILURE;
                        }
                        break;
                    }
                default: break;
            } break;
            case SDL_EVENT_KEY_UP:
            {
                // mod keys
                switch (event->key.scancode) { 
                    case SDL_SCANCODE_B:
                        app->IsBlackModDown = false;
                        break;
                    case SDL_SCANCODE_W:
                        app->IsWhiteModDown = false;
                        break;
                    case SDL_SCANCODE_LSHIFT:
                        app->IsFastScrollDown = false;
                        break;
                    default: break;
                }
            } break;
        } break;
        case SDL_EVENT_MOUSE_WHEEL:
        {
            if (app->IsWhiteModDown || app->IsBlackModDown) {
                SetCheckmate(app, app->IsWhiteModDown ? Side::White : Side::Black, (app->CheckmateMoves + (int) event->wheel.y));
            } else {
                app->IsCheckmate = false;
                ResetAdjustmentTime(app);

                if (app->IsFastScrollDown) {
                    SetEval01(app, (app->TrueEval.Eval01 + (event->wheel.y * (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -0.1f : 0.1f))));
                } else {
                    SetEval01(app, (app->TrueEval.Eval01 + (event->wheel.y * (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -0.01f : 0.01f))));
                }
            } break;
        }
    }
    return SDL_APP_CONTINUE;
}
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    (void) appstate;
    (void) result;
}
