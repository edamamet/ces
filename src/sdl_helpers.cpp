#include "./sdl_helpers.h"
#include "SDL3/SDL_render.h"
#include "SDL3_ttf/SDL_ttf.h"

void SetRenderDrawColor(SDL_Renderer *renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void SetTextColor(TTF_Text *text, SDL_Color color) {
    TTF_SetTextColor(text, color.r, color.g, color.b, color.a);
}
