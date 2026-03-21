# Palomar

Palomar[^1] is a custom GPU renderer provided for the course CSE 169 at UCSD that can be used to complete projects.

# Building

Palomar does not require you to download any external libraries (hopefully). While we do rely on 3rdparty libraries, we try to only use lightweight header-only ones that can be easily included and do not complicate the build. To clone the codebase, do
```sh
git clone https://github.com/mbizzigotti/palomar
```

We use [nob.h](https://github.com/tsoding/nob.h) as our build system. To setup the build we need to bootstrap the build program, so do the following from the `palomar` directory

```sh
# MacOS / Linux
clang build.c -o build
# Windows
clang build.c -o build.exe
```

> [!NOTE]
> Building requires **clang** to be installed;
> on Windows, the easiest way to install **clang** is from [github.com/llvm/releases/latest](https://github.com/llvm/llvm-project/releases/latest) as `LLVM-X.Y.Z-win64.exe`
>
> But, if you already have Visual Studio installed, then it is possible to compile directly using MSVC. To do this, open "x64 Native Tools Command Prompt for VS" and do
> ```sh
> cl /std:c++20 lib/windows/vulkan.lib src/main.cpp /Fe:palomar.exe
> ```
> `build.c` is simplified to only compile with `clang++`, but it can be modified to use MSVC instead!
> **Don't forget to compile the shader!!**

Now that the build program is compiled, there is no need to recompile it, ever[^2]. To then build Palomar, simply run the build program
```sh
./build
```

> [!NOTE]
> If you have an anti-virus software installed, the build program will not work; you will need to whitelist it or disable your anti-virus!

# Running
Try running one of the sample scenes provided!
```sh
./palomar scenes/multiple.json
```
## Camera
Press `TAB` to toggle between viewer camera and first person camera
### Viewer
- Hold and drag with the `LEFT MOUSE` to rotate camera view
- Use `SCROLL WHEEL` to adjust camera distance
### First Person
- Press `ESCAPE` to toggle viewing using mouse movement
- Use `W` `A` `S` `D` to move
- Use `Q` and `E` to move Up and Down (or `SHIFT` and `SPACE`)

# Acknowledgement
The renderer is inspired by [lajolla](https://github.com/BachiLi/lajolla_public/).

We use [Vulkan](https://www.vulkan.org/) as the Graphics API.

We use [cJSON](https://github.com/DaveGamble/cJSON) to parse JSON files.

We use [RGFW](https://github.com/ColleagueRiley/RGFW) for Window creation.

We use [stb_easy_font](https://github.com/nothings/stb) for text rendering.

[^1]: Tzu Mao Li likes to name his programs for UCSD after places in San Diego, I follow this same idea to name this project after a location that is close to where I live, Palomar Mountain. You should visit if you ever get a chance :)
[^2]: Read the source code of `build.c` and `nob.h` to see why our build program never needs to explicitly be recompiled!
