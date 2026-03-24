# Overview

Generally, palomar works by:
- Loading a scene file
- Loading all objects in the scene file
- Allocating required memory
- Sending data to GPU
- Enter update loop

Loading can be viewed at [scene.cpp](../src/scene.cpp)
```cpp
Result Scene::load(string filename, Graphics &gfx) {
    // ... read and parse scene file ...

    // Load each object
    for(auto object: scene_objects) {
        auto loader = create_object_loader(j_object);
        add_object(loader->load());
    }

    // Allocate GPU memory
    gfx.allocate_required_memory();

    // Write out to GPU memory
    for (auto loader: loaders) {
        loader->write_buffers(gfx);
    }
}
```

And the main update loop can be viewed at [engine.cpp](../src/engine.cpp)
```cpp
Result Engine::run() {
    // Continue to render frames until window closes
	while (!RGFW_window_shouldClose(window) && running)
	{
		RGFW_event event;
		while (RGFW_window_checkEvent(window, &event)) {
            // ... handle window events (e.g. mouse move) ...
		}

        // Update and render
		graphics.prepare_frame();
		scene.update_and_render(graphics, dt);
		graphics.submit_frame();
	}
}
```

# Object Loading

Object loading is done in [object_loader.cpp](../src/loaders/object_loader.cpp), where it will pick the right loader to use based on object's type.

Some files to note:
- [mesh_loader.cpp](../src/loaders/mesh_loader.cpp): Loads meshes from `.obj` files, great example of loading from a file.
- [tokenizer.h](../src/loaders/tokenizer.h): Utility provided to tokenize files. Read [Lexical_analysis](https://en.wikipedia.org/wiki/Lexical_analysis) to learn more.
- [rig_loader.cpp](../src/loaders/rig_loader.cpp): File you will be modifying!

# Scene Files
Here is an example scene file with comments for explaination:

```js
{
    "Camera": {
        "type": "Viewer",    // Starting Camera type
                             // ("Viewer", "First_Person")
        "width": 800,        // Screen width
        "height": 600,       // Screen height
        "fov": 45,           // FOV in degrees
        "distance": 10.0,    // Starting distance from center (Viewer)
        "center": [0,0,0],   // Center of rotation (Viewer)
        "position": [0,0,0], // Starting position (First_Person)
    },

    // Every scenes contains a list of objects.
    "Objects": [
        {
            // Every object must have a type.
            "type": "Mesh",

            // The "Mesh" object type must have a "mesh" file to load.
            // NOTE: All filepaths are relative to the scene file loaction.
            "file": "assets/cube.obj"

            // "Transform" sets an objects model matrix
            "transform": [
                // All transforms available:
                {"scale": [0.15, 0.15, 0.15]},
                {"scale": 1.01},
                {"translate": [-3, 0, 0]}

                // You can implement your own transforms
                //  by modifying `src/loaders/transform_loader.cpp`!
            ]
        },
        {
            // Example of what "Rig" could look like
            //  at the end of completing project 3
            "type": "Rig",
            "skeleton":  "assets/wasp.skel",
            "skin":      "assets/wasp.skin",
            "animation": "assets/wasp.anim",
        }
    ]
}
```
