#pragma once

#include "SDL3/SDL_pixels.h"

constexpr SDL_Color WHITE = {255,255,255,255};
constexpr SDL_Color BLACK = {54,51,47,255};
constexpr SDL_Color DEBUG_BG = {25,25,25,255};
constexpr SDL_Color DEBUG_FG = {0,255,255,255};

constexpr const char* FONT_REGULAR_PATH = "asset/font/Zalando_Sans_Expanded/ZalandoSansExpanded-VariableFont_wght.ttf";
constexpr const char* FONT_ITALIC_PATH = "asset/font/Zalando_Sans_Expanded/ZalandoSansExpanded-Italic-VariableFont_wght.ttf";
constexpr int FONT_SIZE = 20;
constexpr float MAX_SCORE = 10.0f;

constexpr double MAX_ADJUSTMENT_TIME = 4;
