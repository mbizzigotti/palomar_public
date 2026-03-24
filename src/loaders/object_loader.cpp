#include "object_loader.h"
#include "transform_loader.h"
#include "mesh_loader.h"
#include "rig_loader.h"

#include "../3rdparty/cJSON.h"

std::unique_ptr<ObjectLoader> create_object_loader(cJSON *object) {
    cJSON *type = cJSON_GetObjectItem(object, "Type");
    if (!cJSON_IsString(type) || (type->valuestring == nullptr)) {
        printf("Warning: Object missing a type field, skipping..\n");
        return nullptr;
    }

    string_view type_string = string_view(type->valuestring);
    if (type_string == "Mesh") {
        return std::make_unique<MeshObjectLoader>();
    }
    if (type_string == "Rig") {
        return std::make_unique<RigObjectLoader>();
    }

    printf("Warning: Unknown object type \"%s\", skipping..\n", type->valuestring);
    return nullptr;
}
