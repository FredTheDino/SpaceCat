#include <stdarg.h>
#include <stdio.h>
#include <stb_image.h>

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "math/block_math.h"

#include "asset/asset.h"
#include "fog_assets"

#include "renderer/text.h"

#include "util/debug.cpp"
#include "util/types.h"
#include "util/memory.h"
#include "util/performance.h"
#include "platform/input.h"
#include "renderer/command.h"
#include "renderer/camera.h"
#include "logic/logic.h"
#include "logic/logic.cpp"
#define OPENGL_RENDERER
#define OPENGL_TEXTURE_WIDTH 512
#define OPENGL_TEXTURE_HEIGHT 512
// NOTE(ed): Chose 256 b.c required by the OpenGL 3.0 spec to be valid.
#define OPENGL_TEXTURE_DEPTH 256
#define SDL

#include "util/io.cpp"
#include "util/memory.cpp"
#include "platform/input.cpp"
#include "renderer/command.cpp"
#include "renderer/text.cpp"
#include "asset/asset.cpp"
#include "util/performance.cpp"

// TODO(ed): Better place for this.
Input::Mapping mapping = {};

#ifdef SDL
// TODO(ed): Better job of abstracting out SDL.
#include "platform/input_sdl.cpp"
#else
#error "No other platform layer than SDL supported."
#endif

#include <ctime>
u64 Perf::highp_now() {
    timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (tp.tv_sec * 1000000000 + tp.tv_nsec) / 1000;
}

#include "math.h"
f32 rand_real() { return ((f32) rand() / (f32) RAND_MAX) * 2.0 - 1.0; }

#include "boilerplate/lines_on_a_grid.cpp"
#ifndef FOG_GAME
#   error "No game found"
//
// To make a game, create the functions:
//
#endif

int main(int argc, char **argv) {
    Util::do_all_allocations();
    ASSERT(Renderer::init("Hello there", 500, 500),
           "Failed to initalize renderer");
    Asset::load("data.fog");
    ASSERT(Logic::init(), "Failed to initalize logic system");

    using namespace Input;
    CHECK(add(&mapping, K(ESCAPE), Player::P1, Name::QUIT),
          "Failed to create mapping");
    Game::setup_input();

    for (int i = 0; i < 1; i++) {
        Function callback = [i](f32 percent, f32 time, f32 delta) {
            LOG("%f", percent);
        };
        Logic::add_callback(Logic::At::PRE_UPDATE, callback, 1.0, 5.0, 1.0);
        Logic::add_callback(Logic::At::PRE_DRAW, callback, 1.2, 5.0, 1.0);
        Logic::add_callback(Logic::At::POST_DRAW, callback, 1.4, 5.0, 1.0);
    }

    Function callback = [](){
        LOG_MSG("I do stuff every third second!");
    };
    Logic::add_callback(Logic::At::POST_UPDATE, callback, 1.0, Logic::FOREVER, 3.0);


    f32 last_tick = SDL_GetTicks() / 1000.0f;
    while (SDL::running) {
        f32 tick = SDL_GetTicks() / 1000.0f;
        f32 delta = tick - last_tick;
        last_tick = tick;

        Perf::report();
        Perf::clear();
        START_PERF(MAIN);
        START_PERF(INPUT);
        frame(&mapping);
        STOP_PERF(INPUT);
        SDL::poll_events();

        if (value(&mapping, Player::ANY, Name::QUIT)) {
            SDL::running = false;
        }

        Logic::call(Logic::At::PRE_UPDATE, tick, delta);
        Game::update(delta);
        Logic::call(Logic::At::POST_UPDATE, tick, delta);

        START_PERF(RENDER);
        Renderer::clear();

        // User defined
        Logic::call(Logic::At::PRE_DRAW, tick, delta);
        Game::draw();
        Logic::call(Logic::At::POST_DRAW, tick, delta);

        Renderer::blit();
        STOP_PERF(RENDER);
        STOP_PERF(MAIN);
    }

    return 0;
}
