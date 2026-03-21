#include "engine.h"
#include "renderers/text_renderer.h"

Options::Options(int argc, char *argv[]) {
	for (int i = 1; i < argc; ++i) {
		char *arg = argv[i];

		// Parse option
		if (arg[0] == '-') {
			printf("Warning: Option not supported \"%s\"\n", arg);
			continue;
		}

		// Parse scene filename
		if (scene_filename) {
			printf("Warning: Should only be one input scene filename!\n");
			continue;
		}
		scene_filename = arg;
	}
}

Result Engine::setup(Options &options) {
	if (graphics.setup(options.enable_graphics_validation))
		return ERROR("Failed to setup graphics context!");

	text.setup(graphics);

	if (scene.load(options.scene_filename, graphics))
		return ERROR("Failed to load scene!");

	text.write_buffers(graphics);

	if (scene.camera.width == 0 || scene.camera.height == 0) {
		printf("Warning: Camera width & height should not be zero, reseting..\n");
		scene.camera = Camera::get_default();
	}

	int width = scene.camera.width, height = scene.camera.height;
	RGFW_windowFlags flags = RGFW_windowCenter | RGFW_windowNoResize;
	window = RGFW_createWindow(options.scene_filename, 0, 0, width, height, flags);
	if (graphics.attach(window))
		return ERROR("Failed to attach to window!");
	
	// Sometimes the window doesn't focus correctly,
	// hopefully this will fix that..
	RGFW_window_focus(window);
	return Success;
}

Result Engine::run() {
	while (!RGFW_window_shouldClose(window) && running)
	{
		RGFW_event event;
		while (RGFW_window_checkEvent(window, &event)) {
			if (event.type == RGFW_quit) {
				running = false;
				break;
			}
			if (scene.camera.handle_event(event))
				continue;
		}

		float dt = timer.seconds_elapsed_and_reset();

		if (graphics.prepare_frame())
			return ERROR("Failed to prepare frame!");

		text.add_text(WHITE, "%.1f FPS", 1.0f / dt);

		scene.update_and_render(graphics, dt);
		text.render(graphics);

		if (graphics.submit_frame())
			return ERROR("Failed to submit frame!");
	}

	RGFW_window_close(window);
	return Success;
}
