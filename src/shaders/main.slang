#include "shared.inl"
#define FONT_SCALE 2.0

struct VSOut {
    float4 screen_position : SV_Position;
    float4 param1;
    float4 param2;

    property float3 normal {
        get { return param1.xyz; }
        set { param1.xyz = newValue; }
    }
    property float3 world_position {
        get { return param2.xyz; }
        set { param2.xyz = newValue; }
    }
};

[[vk::push_constant]] ConstantBuffer<PerDraw> draw;
[[vk::binding(0)]] ConstantBuffer<Scene> scene;
[[vk::binding(1)]] ByteAddressBuffer vertex_buffer;
// [[vk::binding(2)]] StructuredBuffer<float4x4> joint_transforms;

[shader("vertex")]
VSOut main(uint vertexIndex : SV_VertexID)
{
    VSOut output;

    switch (draw.shader_id) {
    case Shader.Position_Normal: {
        const uint index = vertexIndex * sizeof(PNVertex) + draw.vertex_offset;
        PNVertex vertex = vertex_buffer.LoadByteOffset<PNVertex>(index);
        float4 world_position = mul(draw.model, float4(vertex.position, 1));
        output.screen_position = mul(scene.view_proj, world_position);
        output.world_position = world_position.xyz;
        output.normal = mul(float3x3(draw.model), vertex.normal);
    } break;

    case Shader.Text: {
        const uint index = vertexIndex * sizeof(TextVertex) + draw.vertex_offset;
        TextVertex vertex = vertex_buffer.Load<TextVertex>(index);
        float2 scale = FONT_SCALE / scene.screen_size;
        output.screen_position = float4(vertex.position.xy * scale - 1, 0, 1);
        output.param1 = unpackUnorm4x8ToFloat(vertex.color);
    } break;
    }
    return output;
}

float3 eval_point_light(PointLight light, float3 position, float3 normal)
{
    // Ambient
    float3 ambient = light.ambient_strength * light.color;

    // Diffuse
    float3 dir_to_light = normalize(light.position - position);
    float diff = max(dot(normal, dir_to_light), 0);
    float3 diffuse = diff * light.color;

    // Specular
    float3 viewDir = normalize(scene.view_pos - position);
    float3 reflectDir = reflect(-dir_to_light, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0), 32);
    float3 specular = light.specular_strength * spec * light.color;

    return ambient + diffuse + specular;
}

static const PointLight lights[] = {
    {
        { 0, 6, 10 }, /* Position */
        { 1, 1, 1 },  /* Color */
        0.1, 0.5,     /* Ambient & Specular strength */
	},
};

[shader("fragment")]
float4 main(VSOut input) : SV_Target
{
    switch (draw.material_id) {
    case Material.Phong: {
        float3 normal = normalize(input.normal);
        float3 color = 0;

        [unroll]
        for (int i = 0; i < lights.getCount(); ++i) {
            color += eval_point_light(lights[i], input.world_position, normal);
		}
		
        return float4(color, 1);
    }

    case Material.Text: {
        return input.param1;
    }
    }
    return float4(0);
}
