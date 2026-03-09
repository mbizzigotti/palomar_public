#pragma once
#define RGFWDEF
#define RGFW_VULKAN      // Let RGFW include Vulkan for us
#include "3rdparty/RGFW.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "3rdparty/glm/glm.hpp" // RIP compile times :(
#include "3rdparty/glm/ext.hpp"
#include <cstdio>        // Used for file IO
#include <vector>        // Used for Dynamic arrays
#include <chrono>        // Used for Timers
#include <cassert>       // Used for Assertions
#include <cstdarg>       // Used for Variadic Arguments (https://en.cppreference.com/w/c/variadic.html)
#include <utility>       // Used for `std::forward()`
#include <optional>      // Used for `std::optional`
#include <iostream>      // Used for standard output
#include <string_view>   // Used for `std::string_view`
#include <unordered_map> // Used for Hash maps
#include <filesystem>    // Used for filesystem access

// Win32 is the worst C api ever
// What is going on here?
// Win32 is leaking macros and polluting the global namespace
// thanks microsoft 👏👏👏 👍
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
#ifdef ERROR
#undef ERROR
#endif

//! @brief Used to mark all global variables
#define global

//! @brief Log an error with the current file and line number
#define ERROR(...) Error(string(__FILE__), __LINE__, __VA_ARGS__)

// Make vec3, vec4, mat4, ... available to the global namespace,
// because these types are useful and are used often.
using namespace glm;

// `optional` is also a very useful type
using std::optional;
using std::filesystem::path; // .. also file paths

// !!! IMPORTANT !!!
// Using string_view as the default string type
using string = std::string_view;

// NOTE: Prefer to use `enum class`, however this enum
//       is so fundamental that it makes more sense to
//       keep it as a regular enum.
enum Result {
    Success = 0,
    Failed  = 1,
};

struct Color {
    uint8_t r, g, b, a;
};

static constexpr Color WHITE = { 255, 255, 255, 255 };
static constexpr Color RED   = { 255,   0,   0, 255 };
static constexpr Color GREEN = {   0, 255,   0, 255 };
static constexpr Color BLUE  = {   0,   0, 255, 255 };

// NOTE: TIP! Try NOT to use constructors/destructors.
//       It complicates things to a degree that just
//       makes C++ more complicated for no actual gain. 
//       But, DO use destructors as a `defer`, like this! 
template <typename T>
struct AutoFree : public T {    
    ~AutoFree() {
        if (this->data()) {
            free((void*)this->data());
        }
    }
};

// These literals are soo useful, they should just be built into C++,
//   but instead we have to wait years for completely useless features
//   that nobody asks for or even wants.
constexpr u64 operator""_KiB(u64 kilobytes) {
    return kilobytes * 1024;
}
constexpr u64 operator""_MiB(u64 megabytes) {
    return megabytes * 1024 * 1024;
}

//! @brief Helper when inputing strings into printf
//! @example ``` string world = "World";                     ```
//!          ``` printf("Hello %.*s", FORMAT_STRING(world)); ```
#define FORMAT_STRING(STRING) (int)((STRING).size()), (STRING).data()

template <typename T = u8>
struct FixedArenaAllocator {
    u64 count    { 0 }; // How many objects are currently allocated.
    u64 capacity { 0 }; // How many objects can be stored.
    T  *data     { nullptr };

    FixedArenaAllocator(u64 capacity):
        capacity(capacity),
        data((T*)(malloc(sizeof(T) * capacity)))
    {}

    T* allocate(u64 add_count) {
        assert(count + add_count <= capacity);
        T *out = data + count;
        count += add_count;
        return out;
    }

	void free_all() {
		count = 0;
	}
};

using ScratchArena = FixedArenaAllocator<u8>;

global ScratchArena scratch_arena { 64_KiB };

constexpr bool is_power_of_two(u64 num) {
    return ((num) & (num - 1)) == 0; // Think about why this is true!
}

constexpr u64 align(u64 current, u64 alignment) {
    // Non-power-of-two alignment doesn't make much sense and probably
    //   means that something went wrong..
    assert(is_power_of_two(alignment));
    return (current + alignment - 1) & ~(alignment - 1);
}

struct Timer {
    using clock = std::chrono::high_resolution_clock;
    clock::time_point last { clock::now() };
    float tick() {
        clock::time_point now = clock::now();
        auto duration_count = (now - last).count();
        last = now;
        // `duration_count` will be in milliseconds, so convert to seconds
        return (float)(double(duration_count) / 1e9);
    }
};


/**!
 * @brief Log an error marked at a specific file and line number.
 * @param format Printf-style format string
 * @note An error will be logged only the first time this function is called!
*/
Result Error(string file, int line, const char *format, ...);

/**
 * Template magic to have better print functions!
 * 
 * If you want to understand what is happening here:
 *   Search for "C++ Template Parameter Packs"
 */
template <typename T>
void print(const T &value) {
    std::cout << value;
}
template <typename T, typename...P>
void print(const T &obj, P&&...args) {
    print(obj);
    putchar(' ');
    print(std::forward<P>(args)...);
}
template <typename...P>
void println(P&&...args) {
    print(std::forward<P>(args)...);
    putchar('\n');
}

// Print Function Specializations
template<> void print<struct Token>(const struct Token &token);

/**!
 * @brief Loads the contents of an entire file into CPU memory.
 * @param filename Path to the file to load.
 * @return String containing the file contents, or emtpy optional if file could not be loaded.
 * 
 * @note If the file was successfully loaded, then it is the caller's responsiblility to free the string's memory with `free`
*/
optional<string> read_entire_file(string filename);

// Generated by ChatGPT, not sure how good this actually is...
uint64_t hash3(uint64_t a, uint64_t b, uint64_t c) {
    uint64_t h = 0x9e3779b97f4a7c15ULL;  // large odd constant

    h ^= a + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    h ^= b + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    h ^= c + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);

    return h;
}

// NOTE: Need to hash an ordered pair of 3 integers for OBJ loading
template<> struct std::hash<uvec3> {
    std::size_t operator()(const uvec3 &input) const noexcept {
        return hash3(input.x, input.y, input.z);
    }
};
