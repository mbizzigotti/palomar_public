#pragma once
#include "camera.h"
#include "object.h"

struct Graphics;

struct Skybox {
    void render(Graphics &gfx) { (void)gfx; }
};

struct Scene {
    Camera camera;
    Skybox skybox;
    std::vector<Object> objects;

    Result load(string filename, Graphics &gfx);
    void update_and_render(Graphics &gfx, float dt);
};
