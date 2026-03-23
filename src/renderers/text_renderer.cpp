#include "text_renderer.h"
#include "../graphics.h"

// NOTE: As far as I know, this is the easiest way of displaying text
//       on the screen in these types of low level applications.
#include "../3rdparty/stb_easy_font.h"

TextRenderer g_text;

void TextRenderer::setup(Graphics &gfx) {
	vertex_offset = gfx.allocate_buffer(Graphics::VERTEX_BUFFER, MAX_QUAD_COUNT * 4 * sizeof(Vertex));
	index_offset = gfx.allocate_buffer(Graphics::INDEX_BUFFER, MAX_QUAD_COUNT * 6 * sizeof(uint32_t));
	index_start = index_offset / sizeof(u32);
	vertices.resize(MAX_QUAD_COUNT * 4);
}

void TextRenderer::add_text(Color color, const char *format, ...) {
	char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

	Vertex* start = vertices.data() + quad_count * 4;
	int remaining_count = (int)(vertices.size() - quad_count * 4);
	if (remaining_count <= 0) {
		printf("Warning: Ran out of memory to store text!\n");
		return;
	}

	quad_count += stb_easy_font_print(4.0f, 4.0f + line_offset, (char*)buffer, &color.r, start, remaining_count * sizeof(Vertex));
	line_offset += 12.0f; // Font height is 12 "pixels"
}

void TextRenderer::write_buffers(Graphics &gfx) {
	// Index buffer is precomputed for Quad primitives
	std::vector<u32> indices;
	for (uint32_t i = 0; i < MAX_QUAD_COUNT; ++i) {
		indices.emplace_back(i*4 + 0);
		indices.emplace_back(i*4 + 1);
		indices.emplace_back(i*4 + 2);
		indices.emplace_back(i*4 + 2);
		indices.emplace_back(i*4 + 3);
		indices.emplace_back(i*4 + 0);
	}
	gfx.write_index_buffer(index_offset, indices.data(), indices.size() * sizeof(u32));
	// Vertex buffer is dynamic
}

void TextRenderer::render(Graphics &gfx) {
	// Write and Flush dynamic vertex buffer
	u64 size = quad_count * 4 * sizeof(Vertex);
	gfx.write_vertex_buffer(vertex_offset, vertices.data(), size);
	gfx.flush_partition(Graphics::VERTEX_BUFFER, vertex_offset, size);

	// Add draw command to command buffer
	VkCommandBuffer cmd = gfx.current_frame.command_buffer;
	VkPipelineLayout layout = gfx.pipeline_layout;
	shader::PerDraw draw = {
		.vertex_offset = vertex_offset,
		.shader_id = shader::Shader::Text,
		.material_id = shader::Material::Text,
	};
	vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(draw), &draw);
	vkCmdDrawIndexed(cmd, quad_count * 6, 1, index_start, 0, 0);

	// Reset state
	quad_count = 0;
	line_offset = 0;
}
