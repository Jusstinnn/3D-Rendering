#pragma once

/**
 * Square.h
 *
 * Static yellow square Model, offset to the side so it never overlaps
 * the rotating Triangle. Owns its own shader program, VAO, and VBOs —
 * shares no GPU resources with Triangle.
 */

#include "Model.h"
#include "Platform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Square : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Square(AAssetManager* assetMgr);
#else
    Square();
#endif
    ~Square() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif
    GLuint programID    = 0;
    GLuint vao          = 0;
    GLuint vboPos       = 0;
    GLuint vboColor     = 0;
    GLint  uRadianAngle = -1;
};
