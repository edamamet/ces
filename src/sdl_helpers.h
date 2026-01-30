#pragma once

#include <SDL3/SDL_render.h>
#include <SDL3_ttf/SDL_ttf.h>

void SetRenderDrawColor(SDL_Renderer* renderer, SDL_Color color);
void SetTextColor(TTF_Text *text, SDL_Color color);
