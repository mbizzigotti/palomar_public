#define RGFW_IMPLEMENTATION
#include "engine.h"

int main(int argc, char *argv[]) {
	Options options(argc, argv);

	if (!options.scene_filename) {
		printf("Error: No scene provided\n");
		Options::print_help_message();
		return 0;
	}

	Engine engine {};
	if (engine.setup(options))
		return Failed;

	return engine.run();
}
