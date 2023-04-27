#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL.h>
#include <SDL_opengl.h>
#include <GL/gl.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MARKER_SIZE 15.0f

#define SCREEN_FPS 60
#define DELTA_TIME_SEC (1.0f / SCREEN_FPS)
#define DELTA_TIME_MS ((Uint32) floorf(DELTA_TIME_SEC * 1000.0f))
#define BEZIER_PROB_THRESHOLD 10.0f

#define BACKGROUND_COLOR 0x181926FF
#define FOREGROUND_COLOR 0xF5A97FFF // marker
#define LAVENDER_COLOR 0xB7BDF8FF // points
#define DARK_LAVENDER_COLOR 0x6778E6FF
#define FLAMINGO 0xF0C6C6FF 

#define HEX_COLOR(hex)          \
    ((hex) >> (3 * 8)) & 0xFF,  \
    ((hex) >> (2 * 8)) & 0xFF,  \
    ((hex) >> (1 * 8)) & 0xFF,  \
    ((hex) >> (0 * 8)) & 0xFF

int check_sdl_code(int code)
{
    if (code < 0)
    {
        fprintf(stderr, "SDL error: %s\n", SDL_GetError());
        exit(1);
    }

    return code;
}

void *check_sdl_ptr(void *ptr)
{
    if (ptr == NULL)
    {
        fprintf(stderr, "SDL error: %s\n", SDL_GetError());
        exit(1);
    }

    return ptr;
}

typedef struct 
{
    float x;
    float y;
} Vec2;

Vec2 vec2(float x, float y)
{
    return (Vec2){x, y};
}

Vec2 vec2_sub(Vec2 a, Vec2 b)
{
    return vec2(a.x - b.x, a.y - b.y);
}

Vec2 vec2_add(Vec2 a, Vec2 b)
{
    return vec2(a.x + b.x, a.y + b.y);
}

Vec2 vec2_add3(Vec2 a, Vec2 b, Vec2 c)
{
    return vec2_add(vec2_add(a, b), c);
}

Vec2 vec2_scale(Vec2 a, float s)
{
    return vec2(a.x * s, a.y * s);
}

float vec2_length(Vec2 a)
{
    return sqrtf(a.x * a.x + a.y * a.y);
}

float lerpf(float a, float b, float p)
{
    return a + (b - a) * p;
}

Vec2 lerpv2(Vec2 a, Vec2 b, float p)
{
    return vec2_add(a, vec2_scale(vec2_sub(b, a), p));
}

#endif
