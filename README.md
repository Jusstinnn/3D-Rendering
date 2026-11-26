# 3D Rendering Assignments

A collection of C++ graphics assignments exploring rendering, transformations,
animation, lighting, and user interaction across desktop, Android, and web.

These assignments were built using **base code provided by my professor**. I
extended the provided framework to complete the rendering features and
assignment requirements.

## Projects

### Assignment 1: Rotating Triangle

A rotating triangle rendered using OpenGL ES 3.0. The project uses a shared
C++ scene codebase that can be built for Windows, Android, and the web.

[View Assignment 1](./Assignment1)

### Assignment 2: Interactive 3D Fan

A 3D table fan made from multiple parts, including a base, pole, hub, and
spinning blades.

The project explores:

- Hierarchical modelling using a matrix stack
- Transformations for positioning and animating individual parts
- Mouse and touch input to control the fan

[View Assignment 2](./Assignment2)

### Assignment 3: Fan Dashboard and Object Selection

An extension of the 3D fan with lighting, an on-screen dashboard, and object
selection.

The project explores:

- Phong lighting and a procedural checkerboard ground
- A heads-up display (HUD) for controlling fan speed and camera zoom
- Ray-based picking to select and highlight individual fan parts
- Separate rendering passes for the 3D scene and the HUD

[View Assignment 3](./Assignment3)

## Technologies

- C++
- OpenGL ES 3.0
- CMake
- Android Studio and Android NDK
- Emscripten and WebGL

## Running the Projects

Each assignment has its own README with the prerequisites and instructions for
building and running it on desktop, Android, and the web.

## Acknowledgements

The base framework and supporting code were provided by my professor as part
of the course. The rendering work in this repository was completed by extending
that provided base.
