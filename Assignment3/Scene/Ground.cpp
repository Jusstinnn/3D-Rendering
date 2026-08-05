/**
 * Ground.cpp
 *
 * One static quad (2 triangles), Phong-lit, with the checkerboard computed
 * entirely in GroundFragment.glsl from the fragment's world-space (x, z) --
 * see Fan.cpp's drawPart() for the same normal-matrix pattern applied here.
 */

#include "Ground.h"
#include "ShaderHelper.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace {

// Interleaved position(3) + normal(3) per vertex, normal straight up.
// Winding matches Fan.cpp's "top face" pattern -- CCW as seen from +Y.
constexpr float kHalf = 60.0f; // must match Ground::kHalfSize
const float kVertices[4 * 6] = {
    -kHalf, -2.8f,  kHalf,   0.0f, 1.0f, 0.0f,
     kHalf, -2.8f,  kHalf,   0.0f, 1.0f, 0.0f,
     kHalf, -2.8f, -kHalf,   0.0f, 1.0f, 0.0f,
    -kHalf, -2.8f, -kHalf,   0.0f, 1.0f, 0.0f,
};

const GLushort kIndices[6] = { 0, 1, 2,   0, 2, 3 };

constexpr float kColorA[3] = {0.85f, 0.85f, 0.85f}; // light tile
constexpr float kColorB[3] = {0.12f, 0.12f, 0.14f}; // dark tile

} // namespace

Ground::~Ground()
{
    if (ibo_)     glDeleteBuffers(1, &ibo_);
    if (vbo_)     glDeleteBuffers(1, &vbo_);
    if (vao_)     glDeleteVertexArrays(1, &vao_);
    if (program_) glDeleteProgram(program_);
}

void Ground::InitModel()
{
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1); // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBindVertexArray(0);

#ifdef PLATFORM_ANDROID
    program_ = ShaderHelper::buildProgramFromAssets(assetMgr, "shader/GroundVertex.glsl", "shader/GroundFragment.glsl");
#else
    program_ = ShaderHelper::buildProgramFromFile("GroundVertex.glsl", "GroundFragment.glsl");
#endif
    locMVP_          = glGetUniformLocation(program_, "MODELVIEWPROJECTIONMATRIX");
    locModel_        = glGetUniformLocation(program_, "MODELMATRIX");
    locNormalMatrix_ = glGetUniformLocation(program_, "NORMALMATRIX");
    locLightPos_     = glGetUniformLocation(program_, "LIGHTPOS");
    locViewPos_      = glGetUniformLocation(program_, "VIEWPOS");
    locTileSize_     = glGetUniformLocation(program_, "TILESIZE");
    locColorA_       = glGetUniformLocation(program_, "COLOR_A");
    locColorB_       = glGetUniformLocation(program_, "COLOR_B");

    transform.TransformInit();
}

void Ground::SetViewProjection(const glm::mat4& view, const glm::mat4& proj)
{
    viewMatrix_ = view;
    projMatrix_ = proj;
    viewPos_ = glm::vec3(glm::inverse(view)[3]);
}

void Ground::SetLightPos(const glm::vec3& lightPos)
{
    lightPos_ = lightPos;
}

void Ground::Render()
{
    glUseProgram(program_);

    glm::mat4 model(1.0f); // the ground never moves
    glm::mat4 mvp = projMatrix_ * viewMatrix_ * model;
    glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(model));

    glUniformMatrix4fv(locMVP_, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(locModel_, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(locNormalMatrix_, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniform3fv(locLightPos_, 1, glm::value_ptr(lightPos_));
    glUniform3fv(locViewPos_, 1, glm::value_ptr(viewPos_));
    glUniform1f(locTileSize_, kTileSize);
    glUniform3fv(locColorA_, 1, kColorA);
    glUniform3fv(locColorB_, 1, kColorB);

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(0));
    glBindVertexArray(0);
}
