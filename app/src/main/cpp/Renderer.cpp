#include "Renderer.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <GLES3/gl3.h>
#include <memory>
#include <vector>
#include <android/imagedecoder.h>

#include "AndroidOut.h"
#include "Shader.h"
#include "Utility.h"
#include "TextureAsset.h"
#include "Eye.h"
#include "Mouth.h"

#define PRINT_GL_STRING(s) {aout << #s": "<< glGetString(s) << std::endl;}

#define PRINT_GL_STRING_AS_LIST(s) { \
std::istringstream extensionStream((const char *) glGetString(s));\
std::vector<std::string> extensionList(\
        std::istream_iterator<std::string>{extensionStream},\
        std::istream_iterator<std::string>());\
aout << #s":\n";\
for (auto& extension: extensionList) {\
    aout << extension << "\n";\
}\
aout << std::endl;\
}

#define BACKGROUNDBLACK 0.0f, 0.0f, 0.0f, 1.0f

// Shaders básicos para modelos (Texturas)
static const char *vertex = R"vertex(#version 300 es
in vec3 inPosition;
in vec2 inUV;
out vec2 fragUV;
uniform mat4 uProjection;
void main() {
    fragUV = inUV;
    gl_Position = uProjection * vec4(inPosition, 1.0);
}
)vertex";

static const char *fragment = R"fragment(#version 300 es
precision mediump float;
in vec2 fragUV;
uniform sampler2D uTexture;
out vec4 outColor;
void main() {
    outColor = texture(uTexture, fragUV);
}
)fragment";

// ============ SHADERS ACTUALIZADOS PARA LOS OJOS ============

// Vertex Shader (Renderer.cpp)
static const char* eyeVertexShader = R"(#version 300 es
precision mediump float;
layout(location = 0) in vec2 aPosition;
uniform vec2 uPosition;
uniform float uScale;
uniform float uEyeOpenness;
out vec2 vLocalPos;
void main() {
    vec2 scaledPos = aPosition * uScale;
    scaledPos.y *= uEyeOpenness;
    gl_Position = vec4(scaledPos + uPosition, 0.0, 1.0);
    vLocalPos = aPosition;
}
)";

static const char* eyeFragmentShader = R"(#version 300 es
precision mediump float;
in vec2 vLocalPos;
out vec4 FragColor;

uniform vec3 uColor;
uniform float uRadius;
uniform float uThickness;

void main() {
    float dist = length(vLocalPos) / uRadius;

    // Si uRadius es muy grande (ej. 100.0), dist será casi 0
    // Esto hará que outerAlpha sea siempre 1.0 para la boca
    float outerAlpha = 1.0 - smoothstep(0.95, 1.0, dist);

    float innerRadius = 1.0 - uThickness;
    float innerAlpha = smoothstep(innerRadius - 0.05, innerRadius, dist);

    float finalAlpha = outerAlpha * innerAlpha;

    // Para la boca, finalAlpha será 1.0. Para los ojos, hará el aro.
    if (finalAlpha < 0.01) discard;

    FragColor = vec4(uColor, finalAlpha);
}
)";

static constexpr float kProjectionHalfHeight = 2.f;
static constexpr float kProjectionNearPlane = -1.f;
static constexpr float kProjectionFarPlane = 1.f;

Renderer::~Renderer() {
    delete leftEye_;
    delete rightEye_;
    delete mouth_;
    if (eyeShaderProgram_) {
        glDeleteProgram(eyeShaderProgram_);
    }

    if (display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context_ != EGL_NO_CONTEXT) {
            eglDestroyContext(display_, context_);
            context_ = EGL_NO_CONTEXT;
        }
        if (surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(display_, surface_);
            surface_ = EGL_NO_SURFACE;
        }
        eglTerminate(display_);
        display_ = EGL_NO_DISPLAY;
    }
}

GLuint Renderer::createEyeShaderProgram() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &eyeVertexShader, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        aout << "Eye Vertex Shader Error: " << infoLog << std::endl;
        return 0;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &eyeFragmentShader, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        aout << "Eye Fragment Shader Error: " << infoLog << std::endl;
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        aout << "Eye Shader Program Link Error: " << infoLog << std::endl;
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void Renderer::initEyes() {
    aout << "=== Initializing Face Components ===" << std::endl;

    eyeShaderProgram_ = createEyeShaderProgram();
    if (eyeShaderProgram_ == 0) return;

    // Grosor configurado en Eye.h/cpp
    float lineThickness = 0.15f;

    // Posiciones iniciales (se actualizan cada frame en renderEyes)
    leftEye_ = new Eye(-0.35f, 0.1f, 0.3f, EyeShape::CIRCLE, lineThickness);
    rightEye_ = new Eye(0.35f, 0.1f, 0.3f, EyeShape::CIRCLE, lineThickness);

    leftEye_->init();
    rightEye_->init();

    mouth_ = new Mouth(0.0f, -0.35f, 0.5f, 0.15f);
    mouth_->init();

    aout << "=== Face components initialized successfully ===" << std::endl;
}

void Renderer::renderEyes() {
    if (eyeShaderProgram_ == 0) return;

    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime_).count();
    lastFrameTime_ = currentTime;

    pet_.update(deltaTime);

    float eyeOpenness = pet_.isBlinking() ? 0.1f : 1.0f;
    float scale = pet_.getScale();
    float yOffset = pet_.getYOffset();

    // Ajuste de separación
    float eyeSpacing = 0.45f;

    // Actualizamos posiciones según la lógica de la mascota
    leftEye_->setPosition(-eyeSpacing, 0.1f + yOffset);
    rightEye_->setPosition(eyeSpacing, 0.1f + yOffset);

    // El metodo draw de Eye ahora maneja internamente los uniformLoc y el binding de VBO
    leftEye_->draw(eyeShaderProgram_, eyeOpenness, scale);
    rightEye_->draw(eyeShaderProgram_, eyeOpenness, scale);

    if (mouth_) {
        float mouthY = -0.35f + yOffset * 0.5f;
        mouth_->setPosition(0.0f, mouthY);
        mouth_->draw(eyeShaderProgram_);
    }
}

void Renderer::render() {
    updateRenderArea();

    if (shaderNeedsNewProjectionMatrix_) {
        float projectionMatrix[16] = {0};
        Utility::buildOrthographicMatrix(
                projectionMatrix,
                kProjectionHalfHeight,
                float(width_) / height_,
                kProjectionNearPlane,
                kProjectionFarPlane);

        shader_->setProjectionMatrix(projectionMatrix);
        shaderNeedsNewProjectionMatrix_ = false;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    // Importante habilitar Blending para que el smoothstep del shader funcione correctamente
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (leftEye_ && rightEye_) {
        renderEyes();
    }

    auto swapResult = eglSwapBuffers(display_, surface_);
    assert(swapResult == EGL_TRUE);
}

void Renderer::initRenderer() {
    constexpr EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    EGLint numConfigs;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

    auto config = *std::find_if(
            supportedConfigs.get(),
            supportedConfigs.get() + numConfigs,
            [&display](const EGLConfig &config) {
                EGLint red, green, blue, depth;
                if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
                    && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
                    && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
                    && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {
                    return red == 8 && green == 8 && blue == 8 && depth == 24;
                }
                return false;
            });

    EGLSurface surface = eglCreateWindowSurface(display, config, app_->window, nullptr);
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);

    auto madeCurrent = eglMakeCurrent(display, surface, surface, context);
    assert(madeCurrent);

    display_ = display;
    surface_ = surface;
    context_ = context;

    width_ = -1;
    height_ = -1;

    shader_ = std::unique_ptr<Shader>(
            Shader::loadShader(vertex, fragment, "inPosition", "inUV", "uProjection"));
    assert(shader_);
    shader_->activate();

    glClearColor(BACKGROUNDBLACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initEyes();
}

void Renderer::updateRenderArea() {
    EGLint width;
    eglQuerySurface(display_, surface_, EGL_WIDTH, &width);

    EGLint height;
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);

    if (width != width_ || height != height_) {
        width_ = width;
        height_ = height;
        glViewport(0, 0, width, height);
        shaderNeedsNewProjectionMatrix_ = true;
    }
}

void Renderer::handleInput() {
    auto *inputBuffer = android_app_swap_input_buffers(app_);
    if (!inputBuffer) return;

    for (auto i = 0; i < inputBuffer->motionEventsCount; i++) {
        auto &motionEvent = inputBuffer->motionEvents[i];
        auto action = motionEvent.action;
        auto pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        auto &pointer = motionEvent.pointers[pointerIndex];
        auto x = GameActivityPointerAxes_getX(&pointer);
        auto y = GameActivityPointerAxes_getY(&pointer);

        switch (action & AMOTION_EVENT_ACTION_MASK) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                pet_.onTouch();
                break;
            case AMOTION_EVENT_ACTION_MOVE: {
                auto currentTime = std::chrono::high_resolution_clock::now();
                float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime_).count();
                for (auto index = 0; index < motionEvent.pointerCount; index++) {
                    pet_.onHold(deltaTime);
                }
                break;
            }
        }
    }
    android_app_clear_motion_events(inputBuffer);
    android_app_clear_key_events(inputBuffer);
}