#include "camera.h"
#include "3rdparty/glm/gtx/euler_angles.hpp"
#include "3rdparty/cJSON.h"

Camera Camera::get_default() {
    return Camera {
        .type = Type::Viewer,
        .viewer = {
			.distance = 10.0f,
			.azimuth = 60.0f,
			.incline = 10.0f,
		},
        .first_person = {
			.position = vec3(0.0f, 0.0f, 10.0f),
			.azimuth = 180.0f,
		},
        .width = 800,
        .height = 600,
        .far = 100.0f,
        .near = 0.1f,
        .fov = 45.0f,
    };
}

bool Camera::handle_event(RGFW_event &event) {
    switch (type) {
        case Type::Viewer:
        {
            static constexpr float MoveSensitivity = 0.5f;
            static constexpr float ZoomSensitivity = 0.05f;

            switch (event.type)
            {
            case RGFW_mousePosChanged:
            {
                float dx = event.mouse.vecX;
                float dy = -event.mouse.vecY;

                if (RGFW_isMouseDown(RGFW_mouseLeft)) {
					viewer.azimuth += dx * MoveSensitivity;
                    viewer.incline = clamp(viewer.incline - dy * MoveSensitivity, -89.9f, 89.9f);
                }
                return true;
            }
            case RGFW_mouseScroll:
            {
                viewer.distance = clamp(viewer.distance * (1.0f + ZoomSensitivity * event.scroll.y), 0.01f, 1000.0f);
                return true;
            }
            }
			return false;
        }

        case Type::First_Person:
        {
            static constexpr float MouseSensitivity = 0.075f;

            switch (event.type)
            {
            case RGFW_keyPressed:
            {
                switch (event.key.value)
                {
                case RGFW_escape:
                {
                    first_person.is_enabled = !first_person.is_enabled;
                    RGFW_window_showMouse(event.key.win, !first_person.is_enabled);
					RGFW_window_captureRawMouse(event.key.win, first_person.is_enabled);
                    return true;
                }
                }
                return false;
            }
            case RGFW_mousePosChanged:
            {
                if (first_person.is_enabled == false)
                    return false;

                float dx = event.mouse.vecX;
                float dy = -event.mouse.vecY;

                first_person.azimuth += dx * MouseSensitivity;
                first_person.incline = clamp(first_person.incline + dy * MouseSensitivity, -89.9f, 89.9f);
                return true;
            }
            }
			return false;
        }
        default:
        {
            return false;
        }
    }
}

void Camera::update(float dt) {
    if (RGFW_isKeyPressed(RGFW_tab))
        type = Type(((int)type + 1) % 2);

    if (type != Type::First_Person)
        return;

    static constexpr float MovementSpeed = 2.0f;

    vec3 direction = eulerAngleY(radians(-first_person.azimuth))
                    * eulerAngleX(radians(-first_person.incline))
                    * vec4(0.0f, 0.0f, 1.0f, 0.0f);

    direction.y = 0.0f; // ignore Y movement
    direction = normalize(direction);

    vec3 tangent = cross(vec3(0.0f, 1.0f, 0.0f), direction);

    if (RGFW_isKeyDown(RGFW_w)) first_person.position += direction * MovementSpeed * dt;
    if (RGFW_isKeyDown(RGFW_s)) first_person.position -= direction * MovementSpeed * dt;
    if (RGFW_isKeyDown(RGFW_a)) first_person.position += tangent * MovementSpeed * dt;
    if (RGFW_isKeyDown(RGFW_d)) first_person.position -= tangent * MovementSpeed * dt;
    if (RGFW_isKeyDown(RGFW_e) || RGFW_isKeyDown(RGFW_space))  first_person.position.y += MovementSpeed * dt;
    if (RGFW_isKeyDown(RGFW_q) || RGFW_isKeyDown(RGFW_shiftL)) first_person.position.y -= MovementSpeed * dt;
}

void Camera::load(cJSON *object) {
    if (!object)
        return;

    cJSON *j_type = cJSON_GetObjectItem(object, "Type");
    if (cJSON_IsString(j_type) && (j_type->valuestring != nullptr)) {
        string type_string = string(j_type->valuestring);
        if (type_string == "Viewer")
            type = Type::Viewer;
        else if (type_string == "First_Person")
            type = Type::First_Person;
    }

    // Parse type specific data

    switch (type) {
        default:
        case Type::Viewer: {
            // TODO load view options
            // distance
            // center
        } break;
        case Type::First_Person: {
            // TODO load first person options
            // position
        } break;
    }

    // Parse options that are common to all cameras

    auto parse_float = [](cJSON *object, const char* name, float& out) {
        cJSON *item = cJSON_GetObjectItem(object, name);
        if (cJSON_IsNumber(item)) {
            out = (float)(item->valuedouble);
        }
    };
    auto parse_u32 = [](cJSON *object, const char* name, u32& out) {
        cJSON *item = cJSON_GetObjectItem(object, name);
        if (cJSON_IsNumber(item)) {
            out = (float)(item->valueint);
        }
    };

    parse_float(object, "Near", near);
    parse_float(object, "Far", far);
    parse_float(object, "Fov", fov);
    parse_u32(object, "Width", width);
    parse_u32(object, "Height", height);
}

mat4 Camera::get_transform(vec3 *view_position) {
    // NOTE: Projection matrix stays the same for all camera types
    float aspect = (float)(width) / (float)(height);
    // NOTE: Also invert Y axis is for Vulkan weirdness
    mat4 projection = scale(vec3(1,-1,1)) * perspective(radians(fov), aspect, near, far);

    switch (type) {
        case Type::Viewer:
        {
            mat4 world = identity<mat4>();
            world[3][2] = viewer.distance;
            world = eulerAngleY(radians(-viewer.azimuth))
                  * eulerAngleX(radians(-viewer.incline))
                  * world;
            if (view_position)
                *view_position = vec3(world[3]);
            mat4 view = inverse(world);
            return projection * view;
        }
        case Type::First_Person:
        {
            if (view_position)
                *view_position = first_person.position;
            vec3 direction = eulerAngleY(radians(-first_person.azimuth))
                           * eulerAngleX(radians(-first_person.incline))
                           * vec4(0.0f, 0.0f, 1.0f, 0.0f);
            vec3 center = first_person.position + direction;
            mat4 view = lookAt(first_person.position, center, vec3(0.0f, 1.0f, 0.0f));
            return projection * view;
        }
    }
}
