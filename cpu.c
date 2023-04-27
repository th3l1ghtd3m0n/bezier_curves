#include "./common.h"

void render_line(SDL_Renderer *renderer, 
                 Vec2 begin, Vec2 end,
                 uint32_t color)
{
    check_sdl_code(
        SDL_SetRenderDrawColor(renderer, HEX_COLOR(color))
    );
    check_sdl_code(
        SDL_RenderDrawLine(
            renderer,
            (int) floorf(begin.x), 
            (int) floorf(begin.y),
            (int) floorf(end.x),
            (int) floorf(end.y)
        )
    );
}

void fill_rect(SDL_Renderer *renderer, Vec2 pos, Vec2 size, uint32_t color)
{
    check_sdl_code(
        SDL_SetRenderDrawColor(renderer, HEX_COLOR(color))
    );

    const SDL_Rect rect = {
        (int) floorf(pos.x),
        (int) floorf(pos.y),
        (int) floorf(size.x),
        (int) floorf(size.y),
    };

    check_sdl_code(SDL_RenderFillRect(renderer, &rect));
}

void render_marker(SDL_Renderer *renderer, Vec2 pos, uint32_t color)
{
    const Vec2 size = vec2(MARKER_SIZE, MARKER_SIZE);
    fill_rect(
        renderer,
        vec2_sub(
            pos, vec2_scale(
                size, 0.5f
            )
        ), 
        size, color
    );
}

int is_p0_on_the_curve(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p0, float threshold)
{
    const float a = p3.x - 2 * p2.x + p1.x;
    const float b = 2 * (p2.x - p1.x);
    const float c = p1.x - p0.x;
    const float dx = b * b - 4.0f * a * c;

    if (dx >= 0.0f)
    {
        const float t1 = (-b + sqrt(dx)) / (2 * a);
        const float t2 = (-b - sqrt(dx)) / (2 * a);
        const float y1 = p1.y + 2 * t1 * (p2.y - p1.y) + t1 * t1 * (p3.y - 2 * p2.y + p1.y);
        const float y2 = p1.y + 2 * t2 * (p2.y - p1.y) + t2 * t2 * (p3.y - 2 * p2.y + p1.y);
        return (0.0f <= t1 && t1 <= 1.0f && fabsf(p0.y - y1) < threshold) || 
               (0.0f <= t2 && t2 <= 1.0f && fabsf(p0.y - y2) < threshold);
    }

    return 0;
}

Vec2 beziern3_sample(Vec2 p1, Vec2 p2, Vec2 p3, float t)
{
    return vec2_add3(
        p1,
        vec2_scale(
            vec2_sub(p2, p1), 
            2.0f * t
        ),
        vec2_scale(
            vec2_add3(
                p3, 
                vec2_scale(p2, -2.0f), 
                p1
            ),
            t * t
        )
    );
}

Vec2 beziern_sample(Vec2 *ps, Vec2 *xs, size_t n, float p)
{
    memcpy(xs, ps, sizeof(Vec2) * n);

    while (n > 1) 
    {
        for (size_t i = 0; i < n - 1; i++) 
        {
            xs[i] = lerpv2(xs[i], xs[i + 1], p);
        }
        n -= 1;
    }

    return xs[0];
}

void render_bezier3_markers(
    SDL_Renderer *renderer, 
    Vec2 *ps, Vec2 *xs,
    size_t n,
    float s,
    uint32_t color
){
    (void) xs;
    if(n >= 3)
    {
        for (float p = 0.0f; p <= 1.0f; p += s)
        {
            render_marker(
                renderer,
                beziern3_sample(ps[0], ps[1], ps[2], p),
                color
            );
        }
    }
}

void render_bezier_markers(
    SDL_Renderer *renderer, 
    Vec2 *ps, Vec2 *xs,
    size_t n,
    float s,
    uint32_t color
){
    for (float p = 0.0f; p <= 1.0f; p += s)
    {
        render_marker(
            renderer,
            beziern_sample(ps, xs, n, p),
            color
        );
    }
}

void render_bezier_curve(
    SDL_Renderer *renderer,
    Vec2 *ps, Vec2 *xs,
    size_t n,
    float s,
    uint32_t color
){
    for (float p = 0.0f; p <= 1.0f; p += s)
    {
        Vec2 begin = beziern_sample(ps, xs, n, p);
        Vec2 end = beziern_sample(ps, xs, n, p + s);
        render_line(
            renderer,
            begin, end,
            color
        );
    }
}

#define PS_CAPACITY 256

Vec2 ps[PS_CAPACITY];
Vec2 xs[PS_CAPACITY];
size_t ps_count = 0;
int ps_selected = -1;

int ps_at(Vec2 pos)
{
    const Vec2 ps_size = vec2(MARKER_SIZE, MARKER_SIZE);
    for (size_t i = 0; i < ps_count; i++)
    {
        const Vec2 ps_begin = vec2_sub(ps[i], vec2_scale(ps_size, 0.5f));
        const Vec2 ps_end = vec2_add(ps_begin, ps_size);
        if (ps_begin.x <= pos.x && pos.x <= ps_end.x &&
            ps_begin.y <= pos.y && pos.y <= ps_end.y)
        {
            return (int) i;
        }
    }
    return -1;
}

int main(void)
{
    check_sdl_code(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window * const window = check_sdl_ptr(
        SDL_CreateWindow(
            "Bezier Curves",
            0, 0,
            SCREEN_WIDTH, SCREEN_HEIGHT, 
            SDL_WINDOW_RESIZABLE
        )
    );

    SDL_Renderer * const renderer = check_sdl_ptr(
        SDL_CreateRenderer(
            window, -1,
            SDL_RENDERER_ACCELERATED
        )
    );

    check_sdl_code(
        SDL_RenderSetLogicalSize(
            renderer,
            SCREEN_WIDTH,
            SCREEN_HEIGHT
        )
    );

    int quit = 0;
    int markers = 1;
    float t = 0.0f;
    float bezier_sample_step = 0.05f;
    Vec2 bezier_prob = vec2(0.0f, 0.0f);
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN: {
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_F1: {
                            markers = !markers; 
                        } break;
                    }
                } break;
                case SDL_MOUSEBUTTONDOWN: {
                    switch (event.button.button)
                    {
                        case SDL_BUTTON_LEFT: {
                            const Vec2 mouse_pos = vec2(
                                event.button.x, 
                                event.button.y
                            );
                            ps_selected = ps_at(mouse_pos);
                            
                            if (ps_selected < 0 && ps_count < PS_CAPACITY)
                            {
                                ps[ps_count++] = mouse_pos;
                            }
                        } break;
                    }
                } break;
                case SDL_MOUSEBUTTONUP: {
                    if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        ps_selected = -1;
                    }
                } break;
                case SDL_MOUSEMOTION: {
                    Vec2 mouse_pos = vec2(
                        event.motion.x, 
                        event.motion.y
                    );
                    bezier_prob = mouse_pos;
                    if (ps_selected >= 0)
                    {
                        ps[ps_selected] = mouse_pos;
                    }
                } break;
                case SDL_MOUSEWHEEL: {
                    if (event.wheel.y > 0) 
                    {
                        bezier_sample_step = fminf(bezier_sample_step + 0.001f, 0.999f);
                    } else if (event.wheel.y < 0)
                    {
                        bezier_sample_step = fmaxf(bezier_sample_step - 0.001f, 0.001f);
                    }
                } break;
            }
        }

        check_sdl_code(
            SDL_SetRenderDrawColor(
                renderer,
                HEX_COLOR(BACKGROUND_COLOR)
            )
        );

        check_sdl_code(
            SDL_RenderClear(renderer)
        );

        if (ps_count >= 1)
        {
            if (markers)
            {
                render_bezier3_markers(
                    renderer,
                    ps, xs, ps_count,
                    bezier_sample_step,
                    FLAMINGO
                );
            } else
            {
                render_bezier_curve(
                    renderer,
                    ps, xs, ps_count,
                    bezier_sample_step,
                    FOREGROUND_COLOR
                );
            }
        }

        for (size_t i = 0; i < ps_count; i++)
        {
            render_marker(renderer, ps[i], LAVENDER_COLOR);
            if (i < ps_count - 1)
            {
                render_line(renderer, ps[i], ps[i + 1], LAVENDER_COLOR);
            }
        }

        if(ps_count >= 3 && is_p0_on_the_curve(ps[0], ps[1], ps[2], bezier_prob, BEZIER_PROB_THRESHOLD))
        {
            render_marker(renderer, bezier_prob, DARK_LAVENDER_COLOR);
        } else 
        {
            render_marker(renderer, bezier_prob, FOREGROUND_COLOR);
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(DELTA_TIME_MS);
        t += DELTA_TIME_SEC;
    }

    SDL_Quit();

    return 0;
}
