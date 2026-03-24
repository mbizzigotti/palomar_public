#pragma once
#include "core.h"

struct Scene;
struct Graphics;

struct Mesh {
    mat4     model;
    uint32_t vertex_offset;
    uint32_t index_start;
    uint32_t index_count;
};

struct Object {
    enum class Type {
        None, Mesh,
    };

    Type type;
    Mesh mesh;

    void update_and_render(Scene& scene, Graphics& gfx, float dt);
};
