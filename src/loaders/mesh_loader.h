#pragma once
#include "object_loader.h"
#include "../graphics.h"

struct Tokenizer;

struct MeshObjectLoader : public ObjectLoader {
    using Vertex = shader::PNVertex;

    uint32_t            vertex_offset;
    uint32_t            index_offset;
    std::vector<Vertex> vertices;
    std::vector<u32>    indices;

public:

    // --- Object Loader required functions ---

    const char* name() override { return "Mesh"; }
    optional<Object> load(cJSON *object, Graphics &gfx) override;
    Result write_buffers(Graphics &gfx) override;

private:
    Result load_from_file(string_view filename);

    struct Obj_LoaderContext {
        std::unordered_map<uvec3, u32> vertex_map;
        std::vector<vec3> positions;
        std::vector<vec2> texcoords;
        std::vector<vec3> normals;
        u32 next_index = 0;
    };

    Result parse_from_obj(string_view filename, string_view contents);
    Result parse_line_obj(Tokenizer &tokenizer, Obj_LoaderContext &context);
};
