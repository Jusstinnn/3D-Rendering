# Assignment 2 — 3D Fan

A hierarchical 3D table fan (base, pole, hub, four spinning blades) built from
a single shared cube, drawn seven times through a push/pop matrix stack. Runs
on Desktop (Windows), Android, and Web (WebAssembly/WebGL) from one shared
`Scene/` codebase.

Tap the fan (click on desktop/web) to toggle it on/off. Click-drag / swipe to
spin it faster, proportional to drag speed; releasing returns it to base speed.

## Prerequisites

- **Desktop:** CMake 3.15+, Visual Studio 2022 (Desktop development with C++)
- **Android:** Android Studio, with NDK (Side by side) and CMake installed via
  SDK Manager -> SDK Tools
- **Web:** Emscripten SDK (`emsdk`), activated in your terminal session, and
  `git` available on PATH (used to fetch GLM headers on first build)

GLM (the matrix/vector math library `Transform.h` is built on) is fetched
automatically by every build target — you do not install anything for it.

## Build & Run — Desktop

From the project root:

```
script_build_and_run.bat
```

This configures, builds, and launches the app automatically (first run
downloads GLFW, GLEW, and GLM — allow a few minutes). Click to toggle the
fan; click-drag to spin it up. Press **ESC** to quit. Re-running the script
always does a clean rebuild.

## Build & Run — Android

1. Open Android Studio
2. **File -> Open**, select the `android/` folder (not the project root)
3. Wait for Gradle sync to finish
4. Click **Run ▶** with an emulator or device selected
5. Tap to toggle the fan; drag to spin it up. `adb logcat -s Fan3D` shows the
   `Fan ON` / `Fan OFF` log lines and drag-velocity readings.

## Build & Run — Web

1. Activate Emscripten in your terminal (once per session):
   ```
   C:\emsdk\emsdk_env.bat
   ```
2. From the project root, in that same terminal:
   ```
   build_web.bat
   ```
   (first run also clones GLM into `external/glm/`, which is gitignored)
3. Serve and open it:
   ```
   cd web
   python -m http.server 8080
   ```
   Then open `http://localhost:8080/` in a browser. Click to toggle the fan;
   click-drag to spin it up.

## Project Layout

```
Scene/             Shared C++ source
  Fan.h / Fan.cpp     the assignment: geometry, VAO, matrix-stack usage,
                      colours, animation, gesture logic
  Transform.h/.cpp    provided matrix stack (push/pop, post-multiply, radians)
  Renderer.h/.cpp     owns the Fan through the Model interface, forwards
                      render/resize/touch calls to it
  Model.h             lifecycle + touch-event interface every Model implements
  Platform.h          per-platform GL headers + LOGI/LOGE logging
  ShaderHelper.h       shader compile/link, from inline source or file/asset
  main.cpp            Desktop/Web entry point + mouse-to-touch mapping
android/            Android Studio / Gradle project
  app/src/main/cpp/NativeTemplate.cpp   JNI bridge to Renderer
  app/src/main/java/.../MainActivity.java   touch forwarding via queueEvent
  app/src/main/assets/shader/FanVertex.glsl, FanFragment.glsl
CMakeLists.txt      Desktop build entry point
build_web.bat/.sh   Web build entry point (Emscripten)
cmake/ImportDependencies.cmake   fetches GLM (+ GLFW/GLEW for Desktop)
```

`build_desktop/`, `web/`, and `external/` are generated/fetched by the build
scripts — not source, not committed or submitted.
