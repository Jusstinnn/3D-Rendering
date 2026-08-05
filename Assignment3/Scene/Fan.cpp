/**
 * Fan.cpp
 *
 * See Fan.h for the part hierarchy and what changed from Quiz 2. This file:
 *   - Builds three meshes once (cube, cylinder, sphere), all fit to a local
 *     [-0.5, 0.5]^3 box so the *exact same* Transform push/pop
 *     translate+scale numbers from the original cube-only fan still work
 *     unchanged -- only which mesh gets bound per part is new. Blades reuse
 *     the cube mesh (same thin-box shape as the original Quiz 2 blades) so
 *     you see a real front/back/side face as each one rotates.
 *   - Advances spinAngle (blades, around local Z, relative to the hub) and
 *     assemblyAngle (the whole fan, around world Y) every frame, both
 *     scaled from the externally-driven speed_ (Part 6).
 *   - Caches every part's model matrix each frame so Scene3D::PickAt() can
 *     ray-test against the exact geometry that was drawn (Part 8).
 */

#include "Fan.h"
#include "ShaderHelper.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <cmath>
#include <cstdio>

namespace {

constexpr float kPi = 3.14159265358979323846f;

// ---- Cube (base) -- same 24-vert, per-face-normal cube as before ----------
void appendCube(std::vector<float>& pos, std::vector<float>& nrm, std::vector<GLushort>& idx)
{
    const float positions[24 * 3] = {
        -0.5f,-0.5f, 0.5f,   0.5f,-0.5f, 0.5f,   0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f, // front
         0.5f,-0.5f,-0.5f,  -0.5f,-0.5f,-0.5f,  -0.5f, 0.5f,-0.5f,   0.5f, 0.5f,-0.5f, // back
        -0.5f,-0.5f,-0.5f,  -0.5f,-0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f,-0.5f, // left
         0.5f,-0.5f, 0.5f,   0.5f,-0.5f,-0.5f,   0.5f, 0.5f,-0.5f,   0.5f, 0.5f, 0.5f, // right
        -0.5f, 0.5f, 0.5f,   0.5f, 0.5f, 0.5f,   0.5f, 0.5f,-0.5f,  -0.5f, 0.5f,-0.5f, // top
        -0.5f,-0.5f,-0.5f,   0.5f,-0.5f,-0.5f,   0.5f,-0.5f, 0.5f,  -0.5f,-0.5f, 0.5f, // bottom
    };
    const float normals[24 * 3] = {
         0,0, 1,  0,0, 1,  0,0, 1,  0,0, 1,
         0,0,-1,  0,0,-1,  0,0,-1,  0,0,-1,
        -1,0, 0, -1,0, 0, -1,0, 0, -1,0, 0,
         1,0, 0,  1,0, 0,  1,0, 0,  1,0, 0,
         0,1, 0,  0,1, 0,  0,1, 0,  0,1, 0,
         0,-1,0,  0,-1,0,  0,-1,0,  0,-1,0,
    };
    const GLushort indices[36] = {
         0, 1, 2,  0, 2, 3,   4, 5, 6,  4, 6, 7,
         8, 9,10,  8,10,11,  12,13,14, 12,14,15,
        16,17,18, 16,18,19,  20,21,22, 20,22,23,
    };
    pos.assign(positions, positions + 24 * 3);
    nrm.assign(normals, normals + 24 * 3);
    idx.assign(indices, indices + 36);
}

// ---- Cylinder (pole) -- radius 0.5, height 1, side faces only -------------
// (caps aren't needed: the pole's ends are hidden inside the base and hub).
void appendCylinder(std::vector<float>& pos, std::vector<float>& nrm, std::vector<GLushort>& idx, int segments)
{
    for (int i = 0; i < segments; ++i) {
        float theta = i * 2.0f * kPi / segments;
        float x = std::cos(theta) * 0.5f;
        float z = std::sin(theta) * 0.5f;
        // top ring vertex, then bottom ring vertex, same outward normal
        pos.insert(pos.end(), { x, 0.5f, z });
        nrm.insert(nrm.end(), { std::cos(theta), 0.0f, std::sin(theta) });
        pos.insert(pos.end(), { x, -0.5f, z });
        nrm.insert(nrm.end(), { std::cos(theta), 0.0f, std::sin(theta) });
    }
    for (int i = 0; i < segments; ++i) {
        int i2 = (i + 1) % segments;
        GLushort top_i  = static_cast<GLushort>(i * 2);
        GLushort bot_i  = static_cast<GLushort>(i * 2 + 1);
        GLushort top_i2 = static_cast<GLushort>(i2 * 2);
        GLushort bot_i2 = static_cast<GLushort>(i2 * 2 + 1);
        // CCW-from-outside winding (verified by hand -- see the quiz write-up).
        idx.insert(idx.end(), { top_i, bot_i2, bot_i });
        idx.insert(idx.end(), { top_i, top_i2, bot_i2 });
    }
}

// ---- Sphere (hub) -- standard UV sphere, radius 0.5 -----------------------
void appendSphere(std::vector<float>& pos, std::vector<float>& nrm, std::vector<GLushort>& idx, int latBands, int lonSegments)
{
    for (int lat = 0; lat <= latBands; ++lat) {
        float theta = lat * kPi / latBands; // 0 (top) .. pi (bottom)
        float sinT = std::sin(theta), cosT = std::cos(theta);
        for (int lon = 0; lon <= lonSegments; ++lon) {
            float phi = lon * 2.0f * kPi / lonSegments;
            float x = sinT * std::cos(phi);
            float y = cosT;
            float z = sinT * std::sin(phi);
            pos.insert(pos.end(), { x * 0.5f, y * 0.5f, z * 0.5f });
            nrm.insert(nrm.end(), { x, y, z });
        }
    }
    for (int lat = 0; lat < latBands; ++lat) {
        for (int lon = 0; lon < lonSegments; ++lon) {
            GLushort a = static_cast<GLushort>(lat * (lonSegments + 1) + lon);
            GLushort b = static_cast<GLushort>(a + lonSegments + 1);
            idx.insert(idx.end(), { a, b, static_cast<GLushort>(a + 1) });
            idx.insert(idx.end(), { static_cast<GLushort>(a + 1), b, static_cast<GLushort>(b + 1) });
        }
    }
}

constexpr float kBaseColor[3]  = {0.78f, 0.78f, 0.82f}; // light gray/silver
constexpr float kPoleColor[3]  = {0.65f, 0.65f, 0.70f};
constexpr float kHubColor[3]   = {0.95f, 0.82f, 0.15f}; // gold
constexpr float kBladeColor[3] = {1.00f, 0.55f, 0.15f}; // orange
constexpr float kAmber[3]      = {1.0f, 0.75f, 0.0f};

} // namespace

Fan::~Fan()
{
    destroyMesh(meshCube_);
    destroyMesh(meshCylinder_);
    destroyMesh(meshSphere_);
    if (program_) glDeleteProgram(program_);
}

void Fan::ToggleSelected(int index)
{
    if (index >= 0 && index < kPartCount) selectedParts_[index] = !selectedParts_[index];
}

void Fan::ClearSelection()
{
    selectedParts_.fill(false);
}

bool Fan::IsSelected(int index) const
{
    return index >= 0 && index < kPartCount && selectedParts_[index];
}

const char* Fan::GetPartName(int index) const
{
    static char nameBuf[16];
    if (index == 0) return "Base";
    if (index == 1) return "Pole";
    if (index == 2) return "Hub";
    if (index >= 3 && index < kPartCount) {
        std::snprintf(nameBuf, sizeof(nameBuf), "Blade %d", index - 2);
        return nameBuf;
    }
    return "";
}

void Fan::buildMesh(MeshBuffers& mesh,
                      const std::vector<float>& positions,
                      const std::vector<float>& normals,
                      const std::vector<GLushort>& indices)
{
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    GLsizeiptr posBytes = static_cast<GLsizeiptr>(positions.size() * sizeof(float));
    GLsizeiptr nrmBytes = static_cast<GLsizeiptr>(normals.size() * sizeof(float));
    glBufferData(GL_ARRAY_BUFFER, posBytes + nrmBytes, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, posBytes, positions.data());
    glBufferSubData(GL_ARRAY_BUFFER, posBytes, nrmBytes, normals.data());

    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(GLushort)), indices.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(posBytes));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBindVertexArray(0);

    mesh.indexCount = static_cast<GLsizei>(indices.size());
}

void Fan::destroyMesh(MeshBuffers& mesh)
{
    if (mesh.ibo) glDeleteBuffers(1, &mesh.ibo);
    if (mesh.vbo) glDeleteBuffers(1, &mesh.vbo);
    if (mesh.vao) glDeleteVertexArrays(1, &mesh.vao);
    mesh = MeshBuffers{};
}

void Fan::InitModel()
{
    std::vector<float> pos, nrm;
    std::vector<GLushort> idx;

    appendCube(pos, nrm, idx);
    buildMesh(meshCube_, pos, nrm, idx);

    pos.clear(); nrm.clear(); idx.clear();
    appendCylinder(pos, nrm, idx, 12);
    buildMesh(meshCylinder_, pos, nrm, idx);

    pos.clear(); nrm.clear(); idx.clear();
    appendSphere(pos, nrm, idx, 8, 12);
    buildMesh(meshSphere_, pos, nrm, idx);

#ifdef PLATFORM_ANDROID
    program_ = ShaderHelper::buildProgramFromAssets(assetMgr, "shader/FanVertex.glsl", "shader/FanFragment.glsl");
#else
    program_ = ShaderHelper::buildProgramFromFile("FanVertex.glsl", "FanFragment.glsl");
#endif
    locMVP_          = glGetUniformLocation(program_, "MODELVIEWPROJECTIONMATRIX");
    locModel_        = glGetUniformLocation(program_, "MODELMATRIX");
    locNormalMatrix_ = glGetUniformLocation(program_, "NORMALMATRIX");
    locPartColor_    = glGetUniformLocation(program_, "PARTCOLOR");
    locLightPos_     = glGetUniformLocation(program_, "LIGHTPOS");
    locViewPos_      = glGetUniformLocation(program_, "VIEWPOS");

    transform.TransformInit();
}

void Fan::SetViewProjection(const glm::mat4& view, const glm::mat4& proj)
{
    viewMatrix_ = view;
    projMatrix_ = proj;
    viewPos_ = glm::vec3(glm::inverse(view)[3]);
}

void Fan::SetLightPos(const glm::vec3& lightPos)
{
    lightPos_ = lightPos;
}

void Fan::SetSpeed(float speed)
{
    speed_ = speed;
}

void Fan::drawPart(int partIndex, const MeshBuffers& mesh, const float color[3])
{
    glUseProgram(program_);

    glm::mat4 model = *transform.TransformGetModelMatrix();
    glm::mat4 mvp   = *transform.TransformGetModelViewProjectionMatrix();
    glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(model));

    const float* partColor = IsSelected(partIndex) ? kAmber : color;

    glUniformMatrix4fv(locMVP_, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(locModel_, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(locNormalMatrix_, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniform3fv(locPartColor_, 1, partColor);
    glUniform3fv(locLightPos_, 1, glm::value_ptr(lightPos_));
    glUniform3fv(locViewPos_, 1, glm::value_ptr(viewPos_));

    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(0));
    glBindVertexArray(0);
}

void Fan::Render()
{
    // Blades pinwheel around the hub (local Z); the whole fan also turns as
    // one rigid body around world-up Y (assemblyAngle) -- see Fan.h.
    spinAngle     += speed_ * kSpeedToBladeDegPerFrame;
    assemblyAngle += speed_ * kSpeedToAssemblyDegPerFrame;
    if (spinAngle     >= 360.0f) spinAngle     -= 360.0f;
    if (assemblyAngle >= 360.0f) assemblyAngle -= 360.0f;

    transform.TransformSetMatrixMode(VIEW_MATRIX);
    transform.TransformLoadMatrix(&viewMatrix_);

    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadMatrix(&projMatrix_);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // Outer turntable spin: rotating first, before any part is pushed,
    // means every part below inherits it through the stack -- this is what
    // makes the base's square footprint visibly turn, not just the blades.
    transform.TransformRotate(glm::radians(assemblyAngle), 0.0f, 1.0f, 0.0f);

    // base (square slab)
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -2.6f, 0.0f);
        transform.TransformScale(1.0f, 0.25f, 1.0f);
        partModelMatrices_[0] = *transform.TransformGetModelMatrix();
        drawPart(0, meshCube_, kBaseColor);
    transform.TransformPopMatrix();

    // pole (cylinder)
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -1.21f, 0.0f);
        transform.TransformScale(0.15f, 2.53f, 0.15f);
        partModelMatrices_[1] = *transform.TransformGetModelMatrix();
        drawPart(1, meshCylinder_, kPoleColor);
    transform.TransformPopMatrix();

    // hub (sphere)
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 0.2f, 0.0f);
        transform.TransformScale(0.45f, 0.45f, 0.45f);
        partModelMatrices_[2] = *transform.TransformGetModelMatrix();
        drawPart(2, meshSphere_, kHubColor);
    transform.TransformPopMatrix();

    // kBladeCount rectangular-box blades (real 3D boxes, same shape as the
    // original Quiz 2 blades), evenly spaced, spinning around the hub's Z axis
    for (int i = 0; i < kBladeCount; ++i) {
        transform.TransformPushMatrix();
            transform.TransformTranslate(0.0f, 0.2f, 0.15f);
            transform.TransformRotate(glm::radians(spinAngle + i * (360.0f / kBladeCount)), 0.0f, 0.0f, 1.0f);
            transform.TransformTranslate(0.0f, 0.55f, 0.0f);
            // Narrower (X) than before but noticeably thicker (Z) -- the old
            // 0.22 x 0.05 cross-section read as a flat card; 0.14 x 0.12 keeps
            // it slim while making the side face wide enough to actually see
            // as it rotates.
            transform.TransformScale(0.14f, 0.8f, 0.12f);
            partModelMatrices_[3 + i] = *transform.TransformGetModelMatrix();
            drawPart(3 + i, meshCube_, kBladeColor);
        transform.TransformPopMatrix();
    }
}
