#pragma once
#include "object_loader.h"

struct RigObjectLoader : public ObjectLoader {
public:

    // --- Object Loader required functions ---

    const char* name() override { return "Rig"; }
    optional<Object> load(cJSON* json, Graphics &gfx) override;
    Result write_buffers(Graphics &gfx) override;
};
