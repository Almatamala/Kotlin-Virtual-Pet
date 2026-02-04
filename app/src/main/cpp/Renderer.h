#ifndef ANDROIDGLINVESTIGATIONS_RENDERER_H
#define ANDROIDGLINVESTIGATIONS_RENDERER_H

#include <EGL/egl.h>
#include <memory>
#include <chrono>
#include <vector>

#include "Shader.h"
#include "Eye.h"
#include "Mouth.h"
#include "Pet.hpp"

struct android_app;

class Renderer {
public:
    /*!
     * @param pApp the android_app this Renderer belongs to, needed to configure GL
     */
    inline Renderer(android_app *pApp) :
            app_(pApp),
            display_(EGL_NO_DISPLAY),
            surface_(EGL_NO_SURFACE),
            context_(EGL_NO_CONTEXT),
            width_(0),
            height_(0),
            shaderNeedsNewProjectionMatrix_(true),
            leftEye_(nullptr),
            rightEye_(nullptr),
            mouth_(nullptr),
            eyeShaderProgram_(0),
            pet_(50.0f),
            totalTime_(0.0f) {
        initRenderer();
        lastFrameTime_ = std::chrono::high_resolution_clock::now();
    }

    virtual ~Renderer();

    /*!
     * Handles input from the android_app.
     *
     * Note: this will clear the input queue
     */
    void handleInput();

    /*!
     * Renders all the models in the renderer
     */
    void render();

    /*!
     * Gets the Pet instance for external access
     */
    Pet& getPet() { return pet_; }

private:
    /*!
     * Performs necessary OpenGL initialization. Customize this if you want to change your EGL
     * context or application-wide settings.
     */
    void initRenderer();

    /*!
     * @brief we have to check every frame to see if the framebuffer has changed in size. If it has,
     * update the viewport accordingly
     */
    void updateRenderArea();

    /*!
     * Initializes the eye rendering system
     */
    void initEyes();

    /*!
     * Renders the animated eyes
     */
    void renderEyes();

    /*!
     * Compiles and links the eye shader program
     */
    GLuint createEyeShaderProgram();

    android_app *app_;
    EGLDisplay display_;
    EGLSurface surface_;
    EGLContext context_;
    EGLint width_;
    EGLint height_;

    bool shaderNeedsNewProjectionMatrix_;

    std::unique_ptr<Shader> shader_;

    // Sistema de ojos animados
    Eye* leftEye_;
    Eye* rightEye_;
    Mouth* mouth_;
    GLuint eyeShaderProgram_;
    Pet pet_;
    float totalTime_;
    std::chrono::high_resolution_clock::time_point lastFrameTime_;
};

#endif //ANDROIDGLINVESTIGATIONS_RENDERER_H