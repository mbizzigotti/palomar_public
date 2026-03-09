// NOTE: I don't use CMake anymore for any of my personal projects,
//       nowadays I only use nob.h. While I think CMake is a good
//       option, and what most companies will actually use, knowing
//       how a compiler works and being able to work directly with
//       the compiler is super useful knowledge.
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

// typical windows L
#ifdef _WIN32
#	define EXT ".exe"
#else
#	define EXT ""
#endif

#if defined(_WIN32)
#	define PLATFORM_DIR "./lib/windows"
#elif defined(__APPLE__)
#	define PLATFORM_DIR "./lib/macos"
#elif defined(__linux__)
#	define PLATFORM_DIR "./lib/linux"
#endif

Cmd cmd;

int main(int argc, char* argv[])
{
	// ************** IMPORTANT *****************************
	// This program only needs to be compiled ONCE.
	// This is because this program uses Go Rebuild Urself™
	//   Technology, thus this program will always attempt
	//   to rebuild itself if this source file is changed.
	// ******************************************************
	GO_REBUILD_URSELF(argc, argv);

	// Compile shader
	cmd_append(&cmd, PLATFORM_DIR "/" "slangc");
	cmd_append(&cmd, "src/shaders/main.slang", "-o", "shader.spv");
	assert(cmd_run(&cmd, 0));

	// Compile Application
	cmd_append(&cmd, "clang++", "-std=c++17"); // Use c++17
	cmd_append(&cmd, "-Wall", "-Wextra"); // Extra warnings
	cmd_append(&cmd, "-Wno-deprecated-declarations"); // .. Who asked?
	cmd_append(&cmd, "-Wno-missing-field-initializers"); // ??? WHY is this a warning!?
	cmd_append(&cmd, "-g"); // Debug flags
	//cmd_append(&cmd, "-O3"); // Optimization Flags
	cmd_append(&cmd, "-L", PLATFORM_DIR); // Add library search path
#if defined(_WIN32)
	cmd_append(&cmd, "-Wno-microsoft-include");
	cmd_append(&cmd, "-Wno-missing-designated-field-initializers");
	cmd_append(&cmd, "-lgdi32");
#elif defined(__APPLE__)
	cmd_append(&cmd, "-framework", "Cocoa", "-framework", "IOKit");
	cmd_append(&cmd, "-Wl,-rpath,@executable_path");
	assert(copy_file("lib/macos/libvulkan.1.dylib", "libvulkan.1.dylib"));
#elif defined(__linux__)
	cmd_append(&cmd, "-lX11", "-lXrandr");
#endif
	cmd_append(&cmd, "-lvulkan"); // Link with Vulkan
	cmd_append(&cmd, "src/main.cpp"); // Add source code
	cmd_append(&cmd, "-o", "palomar" EXT); // Rename output application
	assert(cmd_run(&cmd, 0));
}
