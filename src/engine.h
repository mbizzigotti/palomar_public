#pragma once
#include "graphics.h"
#include "scene.h"

struct Options {
    bool        enable_graphics_validation { false };
    const char* scene_filename             { 0 };

    Options(int argc, char *argv[]);
};

struct Engine {
    Graphics      graphics {};
    RGFW_window * window   { 0 };
    Scene         scene    {};
    Timer         timer    {};
    bool          running  { true };

    Result setup(Options &options);
    Result run();
};
