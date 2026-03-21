#include "mesh_loader.h"
#include "transform_loader.h"
#include "tokenizer.h"
#include "../graphics.h"
#include "../3rdparty/cJSON.h"

optional<Object> MeshObjectLoader::load(cJSON *object, Graphics &gfx) {
	// Get filename from json object
    cJSON *j_filename = cJSON_GetObjectItem(object, "File");
    if (!cJSON_IsString(j_filename) || (j_filename->valuestring == NULL)) {
		ERROR("Mesh object must contain a file to load!");
        return {};
    }

	// Get model matrix from json object
    mat4 model = load_transform(cJSON_GetObjectItem(object, "Transform"));

    if (load_from_file(j_filename->valuestring))
        return {};

    vertex_offset = gfx.allocate_buffer(Graphics::VERTEX_BUFFER, sizeof(Vertex) * vertices.size());
    index_offset = gfx.allocate_buffer(Graphics::INDEX_BUFFER, sizeof(u32) * indices.size());

    return Object {
        .type = Object::Type::Mesh,
        .mesh = {
            .model = model,
            .vertex_offset = vertex_offset,
            .index_start = (uint32_t)(index_offset / sizeof(u32)),
            .index_count = (uint32_t)(indices.size()),
        }
    };
}

Result MeshObjectLoader::write_buffers(Graphics &gfx) {
    gfx.write_vertex_buffer(vertex_offset, vertices.data(), sizeof(Vertex) * vertices.size());
    gfx.write_index_buffer(index_offset, indices.data(), sizeof(u32) * indices.size());
    return Success;
}

Result MeshObjectLoader::load_from_file(string_view filename) {
    optional<std::string> contents = read_entire_file(filename);
    if (!contents.has_value())
        return ERROR("Failed to read file \"%.*s\"", FORMAT_STRING(filename));

    path extension = path(filename).extension();
    if (extension == ".obj") {
        return parse_from_obj(filename, contents.value());
    }
    else {
        return ERROR("Unknown file type \"%s\", for \"%.*s\"",
            extension.generic_string().c_str(), FORMAT_STRING(filename));
    }
}

Result MeshObjectLoader::parse_from_obj(string_view filename, string_view contents) {
    Tokenizer tokenizer { filename, contents, true };
    Obj_LoaderContext context;
    while (tokenizer.has_tokens()) {
        if (parse_line_obj(tokenizer, context))
            return Failed;
        tokenizer.next();
    }
    vertices.resize(context.vertex_map.size());
    for (auto& [face, index]: context.vertex_map) {
        // Obj files are 1-index based, so we need to subtract 1
        u32 v = face.x - 1, vt = face.y - 1, vn = face.z - 1;

        vertices[index].position = context.positions[v];
        vertices[index].normal = context.normals[vn];

        // Not Implemented
        (void)vt;
        (void)context.texcoords;
    }
    return Success;
}

Result parse_face_index_obj(Tokenizer &tokenizer, uvec3 &out) {
    u64 v = 0, vt = 0, vn = 0;

    if (tokenizer.next_int(v))
        return Failed;
    
    // If there is no '/', then we can stop
    if (!tokenizer.peek().is_symbol())
        goto done;

    // get '/' symbol
    if (tokenizer.next().symbol.value != '/')
        return tokenizer.Error("Expected '/' here");

    if (tokenizer.peek().is_number()) {
        // It is allowed to have nothing here,
        //  so only parse if there is a number.
        if (tokenizer.next_int(vt))
            return Failed;
    }

    // If there is no '/', then we can stop
    if (!tokenizer.peek().is_symbol())
        goto done;

    // get '/' symbol
    if (tokenizer.next().symbol.value != '/')
        return tokenizer.Error("Expected '/' here");

    if (tokenizer.next_int(vn))
        return Failed;

done:
    out = { v, vt, vn };
    return Success;
}

Result MeshObjectLoader::parse_line_obj(Tokenizer &tokenizer, Obj_LoaderContext &context) {
    Token tok = tokenizer.get();

    // Skip over blank lines
    if (tok.type == Token::Type::Newline) {
        return Success;
    }

    // Skip over comments
    if (tok.type == Token::Type::Symbol) {
        if (tok.symbol.value != '#')
            return ERROR("what?");
        
        while (!tokenizer.next().is_end_or_newline());
        return Success;
    }

    // Every line should start with an identifier
    if (tok.type != Token::Type::Identifier)
        return ERROR("should be id here");

    string_view id = tok.identifier.value;

    switch (id[0]) {
        // Vertex Attributes
        break;case 'v': {
            double x = 0.0, y = 0.0, z = 0.0;
            char id1 = id.size() > 1 ? id[1] : 0;
            switch (id1) {
                // Vertex Positions
                break;case 0: {
                    if (tokenizer.next_float(x)) return Failed;
                    if (tokenizer.next_float(y)) return Failed;
                    if (tokenizer.next_float(z)) return Failed;
                    context.positions.emplace_back(vec3{x, y, z});
                }
                // Vertex Normals
                break;case 'n': {
                    if (tokenizer.next_float(x)) return Failed;
                    if (tokenizer.next_float(y)) return Failed;
                    if (tokenizer.next_float(z)) return Failed;
                    context.normals.emplace_back(vec3{x, y, z});
                }
				break;default: {
					// Other attributes are just ignored
					while (!tokenizer.next().is_end_or_newline());
				}
                break;
            }
        }
        // Triangles
        break;case 'f': {
            uvec3 i0, i1, i2;
            if (parse_face_index_obj(tokenizer, i0)) return Failed;
            if (parse_face_index_obj(tokenizer, i1)) return Failed;
            if (parse_face_index_obj(tokenizer, i2)) return Failed;

            auto insert_index = [&](uvec3 index) {
                auto result = context.vertex_map.insert({index, context.next_index});
                if (result.second) {
                    context.next_index += 1;
                }
                indices.emplace_back(result.first->second);
            };

            insert_index(i0);
            insert_index(i1);
            insert_index(i2);
        }
        break;default: {
            // Other statements are just ignored
            while (!tokenizer.next().is_end_or_newline());
        }
    }

    return Success;
}
