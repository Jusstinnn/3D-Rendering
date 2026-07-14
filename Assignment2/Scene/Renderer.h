#pragma once

/**
 * Renderer.h
 *
 * Singleton controller — the only thing main.cpp / NativeTemplate.cpp talk to.
 * Owns a collection of Models through the Model interface only; never
 * includes Fan.h directly outside of createModels().
 */

#include "Model.h"
#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Renderer {
public:
    static Renderer& Instance();

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool initializeRenderer();
    void resize(int w, int h);
    void render();

    // Forwarded from MainActivity.java (via queueEvent + JNI) on Android,
    // and from the GLFW mouse callbacks in main.cpp on Desktop/Web.
    void touchDown(float x, float y);
    void touchMove(float x, float y);
    void touchRelease(float x, float y)