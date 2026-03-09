#pragma once
#include "../object.h"

struct cJSON;
struct Graphics;

// NOTE: C++ does not have an `interface` keyword, so to make interfaces,
//       you make a class that has all virtual functions.
struct ObjectLoader {
    virtual const char* name() = 0;
    virtual optional<Object> load(cJSON *object, Graphics &gfx) = 0;
    virtual Result write_buffers(Graphics &gfx) = 0;
    virtual ~ObjectLoader() = default;
};

std::unique_ptr<ObjectLoader> create_object_loader(cJSON *object);
