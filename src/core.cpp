#include "core.h"
#include <fstream>

Result Error(string_view file, int line, const char *format, ...) {
    static Result last_result = Success;

    // Only report the first error that occurs
    if (last_result == Failed)
        return Failed;

    printf("Error: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n See %.*s:%i\n", FORMAT_STRING(file), line);
    return (last_result = Failed);
}

optional<string> read_entire_file(string_view filename) {
	std::ifstream f(std::string(filename), std::ios::binary);
    if (!f.is_open()) return {};

	f.seekg(0, std::ios::end);
    size_t count = f.tellg();
	f.seekg(0, std::ios::beg);
    char *data = (char*)(malloc(count));
    assert(data != 0);
    f.read(data, count);
    if (f.bad() || f.fail()) {
        printf("Warning: Something went wrong when reading file \"%.*s\"!\n", FORMAT_STRING(filename));
    }
	f.close();
    return string(data, count);
}
