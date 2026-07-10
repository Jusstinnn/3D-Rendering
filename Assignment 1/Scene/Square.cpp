#define LOG_TAG "Square"

#include "Square.h"
#include "ShaderHelper.h"

// ---------------------------------------------------------------------------
// Vertex data — offset to the upper-right in clip space, well outside the
// triangle's rotation sweep (max radius ~0.71 from origin), so the two
// shapes never overlap regardless of the triangle's current angle.
// Drawn as a GL_TRIANGLE_FAN: bottom-left -> bottom-right -> top-right -> top-left.
// ---------------------------------------------------------------------------

static const GLfloat kPositions[] = {
    0.63f, 0.23f,
    0.87f, 0.23f,
    0.87f, 0.47f,
    0.63f, 0.47f
};

static const GLfloat kColors[] = {
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f
};

// ---------------------------------------------------------------------------
// Square
// ---------------------------------------------------------------------------

#ifdef PLATFORM_ANDROID
Square::Square(AAssetManager* assetMgr) : mgr(assetMgr) {}
#else
Square::Square() = default;
#endif

Square::~Square()
{
    glDeleteBuffers(1, &vboColor);
    glDeleteBuffers(1, &vboPos);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(programID);
}

void Square::InitModel()
{
    LOGI("Square::InitModel");

    // Reuses the same generic rotate+color-passthrough shader as Triangle —
    // it's a separate compiled GLuint program, not a shared one.
#ifdef PLATFORM_ANDROID
    programID = ShaderHelper::buildProgramFromAssets(mgr, "shader/BlueTriangleVertex.glsl", "shader/BlueTriangleFragment.glsl");
#else
    programID = ShaderHelper::buildProgramFromFile("BlueTriangleVertex.glsl", "BlueTriangleFragment.glsl");
#endif
    if (!programID) { LOGE("Square: could not create program"); return; }

    uRadianAngle = glGetUniformLocation(programID, "RadianAngle");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kPositions), kPositions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &vboColor);
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kColors), kColors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    LOGI("Square::InitModel done");
}

void Square::Render()
{
    // No glClear here — Triangle renders first each frame and clears the
    // buffer once; Square just draws on top of it.
    glUseProgram(programID);
    glUniform1f(uRadianAngle, 0.0f);   // static — no rotation

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void Square::Resize(int /*w*/, int /*h*/)
{
    // Viewport is already set by Triangle::Resize(); nothing shape-specific
    // needed here, but the override exists to satisfy the Model interface
    // explicitly and to leave room for future per-shape resize behavior.
}
