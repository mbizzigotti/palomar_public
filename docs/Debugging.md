# General Debugging

`print` and `println` are functions provided to make printing more sensible, they both take an arbitrary number of arguments.
```cpp
println("value", value); // Example
```

I recommend to always use a graphical debugger (GUI) to debug your programs. Something like [Visual Studio](.) on Windows, or [gf2](https://github.com/nakst/gf) on Linux. Maybe even [Xcode](.).

> [!NOTE]
> There is [launch.json](../.vscode/launch.json) provided for the debugger within vscode, this might be a good option, but I haven't tested it at all.

If you are on Windows, there is an excellent stand-alone debugger available: [raddbg](https://github.com/EpicGamesExt/raddebugger/releases/latest) by Epic Games that is slowly becoming my go-to debugger.

# Graphics Debugging

If you are running on MacOS, then you are stuck with [Xcode](.) as the only viable graphics debugger. Otherwise [RenderDoc](https://renderdoc.org) is the go-to graphics debugger.
 

## Vulkan Validation Layer

Validation layers are enabled by running palomar with the `-v` flag.

Validation layers are an incredibly useful tool for graphics debugging, and one of the biggest benefits of using Vulkan over OpenGL. How the validation layer works, is that it will look at the inputs to every API call (e.g. `vkCmdDraw`) and let you know if anything about the call is incorrect.

Validation layers (enabled with `-v`) will not work unless you have a special environment vairable set. So to use validation layers, do this before running palomar

### Windows
```powershell
set VK_LAYER_PATH=./lib/windows
```
### MacOS
```bash
export VK_LAYER_PATH=./lib/macos
export DYLD_LIBRARY_PATH=./lib/macos
```
### Linux
```bash
export VK_LAYER_PATH=./lib/linux
export LD_LIBRARY_PATH=./lib/linux
```

> [!NOTE]
> If you have the Vulkan SDK installed, then this step *shouldn't* be nesessary. (at least on Windows)

[Reference](https://vulkan.lunarg.com/doc/view/latest/windows/layer_configuration.html)
