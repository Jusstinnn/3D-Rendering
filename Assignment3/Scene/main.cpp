/**
 * main.cpp
 *
 * Desktop and WebGL entry point. The left mouse button is mapped to the same
 * three touch events used by Android: down, move, and release.
 */

#include "Platform.h"
#include "Renderer.h"

#include <iostream>

namespace {
GLFWwindow* windowHandle = nullptr;
bool leftButtonDown = false;

void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
    Renderer::Instance().resize(width, height);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    double x = 0.0;
    double y = 0.0;
    glfwGetCursorPos(window, &x, &y);

    if (action == GLFW_PRESS) {
        leftButtonDown = true;
        Renderer::Instance().touchEventDown(static_cast<float>(x), static_cast<float>(y));
    } else if (action == GLFW_RELEASE) {
        Renderer::Instance().touchEventRelease(static_cast<float>(x), static_cast<float>(y));
        leftButtonDown = false;
    }
}

void cursorPositionCallback(GLFWwindow*, double x, double y)
{
    if (!leftButtonDown) return;
    Renderer::Instance().touchEventMove(static_cast<float>(x), static_cast<float>(y));
}

bool initializeWindow()
{
    if (!glfwInit()) {
        LOGE("glfwInit failed");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    windowHandle = glfwCreateWindow(800, 600, "Programming Quiz 3 - Fan Dashboard HUD", nullptr, nullptr);
    if (windowHandle == nullptr) {
        LOGE("glfwCreateWindow failed");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(windowHandle);
    glfwSwapInterval(1);

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_DESKTOP)
    glewExperimental = GL_TRUE;
    const GLenum glewStatus = glewInit();
    // GLEW may generate GL_INVALID_ENUM when initialized against an ES context.
    glGetError();
    if (glewStatus != GLEW_OK) {
        LOGE("glewInit failed: %s", reinterpret_cast<const char*>(glewGetErrorString(glewStatus)));
        glfwDestroyWindow(windowHandle);
        windowHandle = nullptr;
        glfwTerminate();
        return false;
    }
#endif

    glfwSetFramebufferSizeCallback(windowHandle, framebufferSizeCallback);
    glfwSetMouseButtonCallback(windowHandle, mouseButtonCallback);
    glfwSetCursorPosCallback(windowHandle, cursorPositionCallback);

    if (!Renderer::Instance().initializeRenderer()) {
        LOGE("Renderer initialization failed");
        return false;
    }

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(windowHandle, &width, &height);
    Renderer::Instance().resize(width, height);
    return true;
}

void cleanUp()
{
    Renderer::Instance().shutdown();
    if (windowHandle != nullptr) {
        glfwDestroyWindow(windowHandle);
        windowHandle = nullptr;
    }
    glfwTerminate();
}

#ifdef PLATFORM_EMSCRIPTEN
void webMainLoop()
{
    if (glfwWindowShouldClose(windowHandle)) {
        emscripten_cancel_main_loop();
        cleanUp();
        return;
    }

    glfwPollEvents();
    Renderer::Instance().render();
    glfwSwapBuffers(windowHandle);
}
#endif
} // namespace

int main()
{
    std::cout << "Programming Quiz 3 - Fan Dashboard HUD (GLFW)\n";
    std::cout << "Click the HUD buttons to control speed/zoom.  Click the fan to pick a part.  ESC to quit.\n";

    if (!initializeWindow()) return -1;

#ifdef PLATFORM_EMSCRIPTEN
    emscripten_set_main_loop(webMainLoop, 0, 1);
#else
    while (!glfwWindowShouldClose(windowHandle)) {
        if (glfwGetKey(windowHandle, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(windowHandle, GLFW_TRUE);
        }

        Renderer::Instance().render();
        glfwSwapBuffers(windowHandle);
        glfwPollEvents();
    }
    cleanUp();
#endif

    return 0;
}
