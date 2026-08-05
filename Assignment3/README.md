# Programming Quiz 3 — Fan Dashboard HUD

Extends the Quiz 2 3D Fan with a split renderer: **Scene3D** (the fan + a new
procedural checkerboard ground, Phong-lit) and **SceneHUD** (an on-screen
dashboard, drawn in orthographic screen space) rendered in a fixed order
every frame. Speed and camera zoom are now driven entirely by HUD buttons —
the old tap-to-toggle / drag-to-boost gestures from Quiz 2 are gone. Clicking
directly on the fan (not a HUD button) picks the part under the cursor via a
screen-click → world-ray → closest-hit test, and highlights it. Runs on
Desktop (Windows), Android, and Web (WebAssembly/WebGL) from one shared
`Scene/` codebase.

## Controls

- **+SPD / -SPD** (top-left, blue) — fan rotation speed, 0–20, shown live on
  the 20-block speed bar. Press-and-hold repeats the action (see Bonus below).
- **+ZOOM / -ZOOM** (top-left, purple) — camera distance, clamped to a range
  that keeps the fan on-screen. Also supports press-and-hold.
- **Click/tap the fan directly** (not a HUD button) — picks the part under
  the cursor (base, pole, hub, or an individual blade) and highlights it
  magenta. Multiple parts can be highlighted at once: each click toggles
  just that part, independent of whatever else is already selected. Clicking
  empty space clears every highlight at once.

## Prerequisites

- **Desktop:** CMake 3.15+, Visual Studio 2022 (Desktop development with C++)
- **Android:** Android Studio, with NDK (Side by side) and CMake installed via
  SDK Manager -> SDK Tools
- **Web:** Emscripten SDK (`emsdk`), activated in your terminal session, and
  `git` available on PATH (used to fetch GLM headers on first build). Note:
  the web build is also built and deployed automatically by this repo's
  `pages.yml` GitHub Actions workflow — you don't need a local Emscripten
  install just to see it running live.

GLM (the matrix/vector math library `Transform.h` is built on) is fetched
automatically by every build target — you do not install anything for it.

## Build & Run — Desktop

From the project root:

```
script_build_and_run.bat
```

This configures, builds, and launches the app automatically (first run
downloads GLFW, GLEW, and GLM — allow a few minutes). Click the HUD buttons
to control speed/zoom; click the fan to pick a part. Press **ESC** to quit.
Re-running the script always does a clean rebuild.

## Build & Run — Android

1. Open Android Studio
2. **File -> Open**, select the `android/` folder (not the project root)
3. Wait for Gradle sync to finish
4. Click **Run ▶** with an emulator or device selected
5. Tap the HUD buttons to control speed/zoom; tap the fan to pick a part.
   `adb logcat -s Fan3D` shows `+SPD pressed`, `-ZOOM pressed`, and
   `Pick: <part> selected/deselected` log lines.

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
   Then open `http://localhost:8080/` in a browser. Click the HUD buttons to
   control speed/zoom; click the fan to pick a part.

## Project Layout

```
Scene/             Shared C++ source
  Fan.h / Fan.cpp     the fan: sphere hub / cylinder pole / box blades on the
                      same Transform push/pop stack from Quiz 2, Phong
                      uniforms, per-part model-matrix caching for picking,
                      multi-select highlight state
  Ground.h / Ground.cpp   procedural checkerboard floor, Phong-lit, no texture
  Scene3D.h / Scene3D.cpp   owns Fan + Ground, builds the shared camera/light
                      every frame, PickAt() (unproject click -> world ray ->
                      ray-vs-box test against every pickable part)
  SceneHUD.h / SceneHUD.cpp   4 buttons + 20-block speed bar in one
                      orthographic pass, screen-space hit-testing,
                      press-and-hold repeat
  Renderer.h/.cpp     owns Scene3D + SceneHUD, implements the clear -> 3D
                      (depth on) -> disable depth -> HUD -> enable depth ->
                      swap frame order, routes touch (HUD first, fan pick on
                      a miss)
  Transform.h/.cpp    provided matrix stack (push/pop, post-multiply, radians)
  Model.h             lifecycle + touch-event interface every Model implements
  Platform.h          per-platform GL headers + LOGI/LOGE logging
  ShaderHelper.h       shader compile/link, from inline source or file/asset
  main.cpp            Desktop/Web entry point + mouse-to-touch mapping
android/            Android Studio / Gradle project
  app/src/main/cpp/NativeTemplate.cpp   JNI bridge to Renderer
  app/src/main/java/.../MainActivity.java   touch forwarding via queueEvent
  app/src/main/assets/shader/
    FanVertex.glsl, FanFragment.glsl       Phong (ambient + diffuse + specular)
    GroundVertex.glsl, GroundFragment.glsl Phong + procedural checkerboard
    HUDVertex.glsl, HUDFragment.glsl       flat-coloured screen-space quads
CMakeLists.txt      Desktop build entry point
build_web.bat/.sh   Web build entry point (Emscripten)
cmake/ImportDependencies.cmake   fetches GLM (+ GLFW/GLEW for Desktop)
```

`build_desktop/`, `web/`, and `external/` are generated/fetched by the build
scripts — not source, not committed or submitted.

## Bonus Challenges

- **Press-and-hold.** Holding down any of +SPD / -SPD / +ZOOM / -ZOOM
  applies the button's action once immediately, then keeps repeating it at a
  steady rate (after a short initial delay) for as long as the button stays
  held — no need to click repeatedly to reach a target speed or zoom level.
  Implemented in `SceneHUD::updateHold()`, ticked once per frame from
  `SceneHUD::Render()`.
