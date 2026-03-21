#pragma once
#include "camera.h"
#include "object.h"

struct Graphics;

struct Scene {
    Camera camera;
    std::vector<Object> objects;

    Result load(string filename, Graphics &gfx);
    void update_and_render(Graphics &gfx, float dt);
};
