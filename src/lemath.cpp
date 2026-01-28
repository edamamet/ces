#include "lemath.h"
#include "SDL3/SDL_stdinc.h"

float MoveTowards(float current, float target, float maxDelta) {
    if (maxDelta < 0.0f) maxDelta = 0.0f;
    float delta = target - current;
    if (SDL_fabsf(delta) <= maxDelta) {
        return target;
    }
    return current + maxDelta * (delta > 0.0f ? 1.0f : -1.0f);
}

float Lerp(float a, float b, float t) {
    return b*t + (1-t)*a;
}
