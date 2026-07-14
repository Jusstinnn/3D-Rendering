/**
 * NativeTemplate.cpp
 *
 * Thin JNI bridge between the Java/Android layer and Renderer.
 * All Model/shape logic lives in Scene/Renderer.* + Fan.
 *
 * Native method signatures match MainActivity.java:
 *   package com.example.fan3d
 *   class   MainActivity
 *
 * Symbol-name rule: JNI resolves each Java "native" method to a C symbol
 * named Java_<package_with_underscores>_<Class>_<method>. If the Java
 * package or class name ever changes, every symbol below must be renamed
 * to match exactly, or the app will crash at runtime with
 * UnsatisfiedLinkError even though the code compiles fine.
 */

#define LOG_TAG "Fan3DNative"

#include <jni.h>
#include <android/asset_manager_jni.h>
#include "Platform.h"   // GLES3/gl3.h + LOGI/LOGE — Model.h/Renderer.h deliberately don't pull this in
#include "Renderer.h"

// ---------------------------------------------------------------------------
// JNI entry points
// ---------------------------------------------------------------------------

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_fan3d_MainActivity_nativeInit(
        JNIEnv* env, jobject /*thiz*/, jobject assetManager)
{
    LOGI("nativeInit called");
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    Renderer::Instance().setAssetManager(mgr);
    return Renderer::Instance().initializeRenderer() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeResize(
        JNIEnv* /*env*/