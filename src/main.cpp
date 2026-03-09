#define RGFW_IMPLEMENTATION
#include "engine.h"

int main(int argc, char *argv[]) {
	Options options(argc, argv);

	if (!options.scene_filename) {
		printf("No scene provided\n");
		return 0;
	}

	Engine engine {};
	if (engine.setup(options))
		return Failed;

	return engine.run();
}

// NOTE: This is a well-known compilation technique called a "Unity Build".
//       Read https://en.wikipedia.org/wiki/Unity_build if you want to learn more.

#include "graphics.cpp"
#include "scene.cpp"
#include "engine.cpp"
#include "camera.cpp"
#include "object.cpp"
#include "core.cpp"

#include "loaders/object_loader.cpp"
#include "loaders/tokenizer.cpp"

#include "renderers/text_renderer.cpp"

#define print cJSON__print
#include "3rdparty/cJSON.c"
