#!/usr/bin/env sh
# =============================================================================
# build_web.sh - Build 3D Fan for WebGL using Emscripten
#
# Prerequisites:
#   1. Install the Emscripten SDK:  https://emscripten.org/docs/getting_started/
#   2. Activate it:   source /path/to/emsdk/emsdk_env.sh
#   3. Run from the project root:  ./build_web.sh
#
# Output: web/index.html  (+ index.js, index.wasm, index.data)
# Serve:  cd web && python3 -m http.server 8080
# =============================================================================

set -e

mkdir -p web

# GLM is header-only; fetch it once into external/ (gitignored, not committed)
# instead of pulling in a full CMake configure just for a WebGL build.
if [ ! -d external/glm ]; then
    echo "Fetching GLM (header-only, needed by Transform.h) ..."
    git clone --depth 1 https://github.com/g-truc/glm.git external/glm
fi

emcc -std=c++17 \
    Scene/main.cpp \
    Scene/Renderer.cpp \
    Scene/Fan.cpp \
    Scene/Transform.cpp \
    -IScene \
    -Iexternal/glm \
    -DUSE_GLFW \
    -s USE_GLFW=3 \
    -s USE_WEBGL2=1 \
    -s MIN_WEBGL_VERSION=2 \
    -s FULL_ES3=1 \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    --preload-file android/app/src/main/assets/shader@assets/shader \
    -o web/index.html

echo ""
echo "Build succeeded."
echo "Run:  cd web && python3 -m http.server 8080"
echo "Open: http://localhost:8080/"
