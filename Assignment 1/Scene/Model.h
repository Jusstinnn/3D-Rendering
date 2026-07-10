#pragma once

/**
 * Model.h
 *
 * Lifecycle interface every renderable shape must implement.
 * Renderer owns shapes only through this interface — never a concrete type.
 */

class Model {
public:
    Model() {}
    virtual ~Model() {}

    virtual void InitModel() = 0;              // called once, before the first frame
    virtual void Render() = 0;                  // called once per frame
    virtual void Resize(int /*w*/, int /*h*/) {}  // called whenever the surface/window resizes
};
