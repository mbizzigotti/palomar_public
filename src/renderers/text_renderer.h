#pragma once
#include "../core.h"
#include "../graphics.h"

struct TextRenderer {
	using Vertex = shader::TextVertex;
	static constexpr uint32_t MAX_QUAD_COUNT = (1 << 12);

	u32 vertex_offset;
	u32 index_offset;
	u32 index_start;
	u32 quad_count;
    float line_offset;

	std::vector<Vertex> vertices;

	void add_text(Color color, const char* format, ...);

	void setup(Graphics &gfx);
	void write_buffers(Graphics &gfx);
	void render(Graphics &gfx);
};

global TextRenderer g_text;
