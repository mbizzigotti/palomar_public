#include "scene.h"
#include "graphics.h"
#include "3rdparty/cJSON.h"
#include "loaders/object_loader.h"

Result Scene::load(string filename, Graphics &gfx) {
    optional<string> contents = read_entire_file(filename);
    if (!contents.has_value())
        return ERROR("Failed to load scene file \"%.*s\"", FORMAT_STRING(filename));

    cJSON *j_scene = cJSON_ParseWithLength(contents->data(), contents->size());
    if (!j_scene)
        return ERROR("Failed to parse scene file!");

    // Load camera from scene file
    camera = Camera::get_default();
    camera.load(cJSON_GetObjectItem(j_scene, "Camera"));

    // Temperarily set the current path to the scene's directory when loading
    auto old_path = std::filesystem::current_path();
    std::filesystem::current_path(path(filename).parent_path());

    Result result = Success;
    std::vector<std::unique_ptr<ObjectLoader>> loaders;
    if (cJSON *j_objects = cJSON_GetObjectItem(j_scene, "Objects")) {
        cJSON *j_object = 0;
        cJSON_ArrayForEach(j_object, j_objects) {
            std::unique_ptr<ObjectLoader> loader = create_object_loader(j_object);
            
            // Failed to get object loader, then skip
            if (!loader)
                continue;

            // Here we actually load the object!
            optional<Object> object = loader->load(j_object, gfx);
            if (!object.has_value()) {
                return ERROR("Failed to load object from %s object loader!", loader->name());
            }
            objects.emplace_back(std::move(object.value()));
            
            // We need to keep loaders around in order to write
            // to graphics memory after it is allocated.
            loaders.emplace_back(std::move(loader));
        }
    }

    gfx.allocate_required_memory();

    for (auto& loader: loaders) {
        loader->write_buffers(gfx);
    }

    std::filesystem::current_path(old_path);
    cJSON_Delete(j_scene);
    return result;
}

void Scene::update_and_render(Graphics &gfx, float dt) {
    camera.update(dt);
    for (auto &obj: objects) {
        obj.update_and_render(*this, gfx, dt);
    }
	
    gfx.scene_data.view_proj = camera.get_transform(&gfx.scene_data.view_pos);
    gfx.scene_data.screen_size.x = gfx.image_extent.width;
    gfx.scene_data.screen_size.y = gfx.image_extent.height;
}
