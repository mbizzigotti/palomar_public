# Palomar

Palomar[^1] is a custom GPU renderer provided for the course CSE 169 at UCSD that can be used to complete projects.

# Documentation
- [Building](#building)
- [Running](./docs/Running.md)
- [Walkthrough](./docs/Walkthrough.md)
- [Debugging](./docs/Debugging.md)

# Building

Palomar does not require you to download any external libraries (hopefully). While we do rely on 3rdparty libraries, we try to only use lightweight header-only ones that can be easily included and do not complicate the build. To clone the codebase, do
```sh
git clone https://github.com/mbizzigotti/palomar_public
```

We use [CMake](https://cmake.org) as our build system. To setup the build we can do

```sh
cmake -S . -B build
```

And, to build the project, we can do
```sh
cmake --build build
```

Try running one of the sample scenes provided! On a Unix-like system, you can do
```sh
./build/palomar scenes/cube.json
./build/palomar scenes/sample.json
```

# Acknowledgement
The implementation of this program is inspired by [lajolla](https://github.com/BachiLi/lajolla_public/).

We use [Vulkan](https://www.vulkan.org/) as the Graphics API.

We use [cJSON](https://github.com/DaveGamble/cJSON) to parse JSON files.

We use [RGFW](https://github.com/ColleagueRiley/RGFW) for Window creation.

We use [stb_easy_font](https://github.com/nothings/stb) for text rendering.

[^1]: Tzu Mao Li likes to name his programs for UCSD after places in San Diego, I follow this same idea to name this project after a location that is close to where I live, Palomar Mountain. You should visit if you ever get a chance :)
[^2]: Read the source code of `build.c` and `nob.h` to see why our build program never needs to explicitly be recompiled!
