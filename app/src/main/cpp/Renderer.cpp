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

// ============ SHADERS INTEGRADOS ============

static const char* eyeVertexShader = R"(#version 300 es
precision mediump float;
layout(location = 0) in vec2 aPosition;
uniform vec2 uPosition;
uniform float uScale;
uniform float uEyeOpenness;
uniform bool uIsMouth;

out vec2 vLocalPos;

void main() {
    vec2 scaledPos = aPosition * uScale;
    // Solo aplicamos parpadeo si no es la boca
    if(!uIsMouth) {
        scaledPos.y *= uEyeOpenness;
    }
    gl_Position = vec4(scaledPos + uPosition, 0.0, 1.0);
    vLocalPos = aPosition;
}
)";

static const char* eyeFragmentShader = R"(#version 300 es
precision mediump float;
in vec2 vLocalPos;
out vec4 FragColor;

uniform vec3 uColor;
uniform float uThickness;
uniform bool uIsMouth;
uniform float uEyeOpenness;
uniform float uTime;
uniform bool uVHSEnabled;

uniform float uWidth;
uniform float uHeight;
uniform float uMouthWidth;
uniform float uMouthHeight;

float noise(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233) + uTime * 0.1)) * 43758.5453123);
}

float dot2(vec2 v) { return dot(v,v); }

// Función para la boca :3
float sdBezier(in vec2 pos, in vec2 A, in vec2 B, in vec2 C) {
    vec2 a = B - A; vec2 b = A - 2.0*B + C; vec2 c = a * 2.0; vec2 d = A - pos;
    float kk = 1.0/dot(b,b);
    float kx = kk * dot(a,b);
    float ky = kk * (2.0*dot(a,a)+dot(d,b)) / 3.0;
    float kz = kk * dot(d,a);
    float res = 0.0;
    float p = ky - kx*kx; float p3 = p*p*p;
    float q = kx*(2.0*kx*kx-3.0*ky) + kz;
    float h = q*q + 4.0*p3;
    if(h >= 0.0) {
        h = sqrt(h); vec2 x = (vec2(h,-h)-q)/2.0;
        vec2 uv = sign(x)*pow(abs(x), vec2(1.0/3.0));
        float t = clamp(uv.x+uv.y-kx, 0.0, 1.0);
        res = dot2(d + (c + b*t)*t);
    } else {
        float z = sqrt(-p); float v = acos(q/(p*z*2.0)) / 3.0;
        float m = cos(v); float n = sin(v)*1.732050808;
        vec3 t = clamp(vec3(m+m,-n-m,n-m)*z-kx,0.0,1.0);
        res = min(dot2(d+(c+b*t.x)*t.x), dot2(d+(c+b*t.y)*t.y));
    }
    return sqrt(res);
}

// Función optimizada Quadratic Circle
float sdQuadraticCircle(in vec2 p) {
    p = abs(p); if( p.y>p.x ) p=p.yx;
    float a = p.x-p.y;
    float b = p.x+p.y;
    float c = (2.0*b-1.0)/3.0;
    float h = a*a + c*c*c;
    float t;
    if( h>=0.0 ) {
        h = sqrt(h);
        t = sign(h-a)*pow(abs(h-a),1.0/3.0) - pow(h+a,1.0/3.0);
    } else {
        float z = sqrt(-c);
        float v = acos(a/(c*z))/3.0;
        t = -z*(cos(v)+sin(v)*1.732050808);
    }
    t *= 0.5;
    vec2 w = vec2(-t,t) + 0.75 - t*t - p;
    return length(w) * sign( a*a*0.5+b-1.5 );
}

void main() {
    // 1. Jitter horizontal sutil
    float jitter = uVHSEnabled ? (noise(vec2(uTime, vLocalPos.y * 10.0)) - 0.5) * 0.008 : 0.0;
    vec2 pMod = vLocalPos + vec2(jitter, 0.0);

    float alpha = 0.0;
    float dist = 0.0;

    if (uIsMouth) {
        vec2 p = pMod; p.x = abs(p.x);
        vec2 A = vec2(0.0, 0.0);
        vec2 B = vec2(uMouthWidth * 0.625, -uMouthHeight * 0.5);
        vec2 C = vec2(uMouthWidth * 0.5, 0.0);
        dist = sdBezier(p, A, B, C);
        alpha = 1.0 - smoothstep(uThickness - 0.01, uThickness + 0.01, dist);
    } else {
        vec2 p = pMod / vec2(uWidth, uHeight);
        if(abs(p.y) > uEyeOpenness) discard;
        dist = sdQuadraticCircle(p);
        alpha = 1.0 - smoothstep(-0.02, 0.02, dist);
    }

    vec3 finalColor = uColor;

    if (uVHSEnabled) {
        float grain = noise(vLocalPos * 3.0) * 0.05;

        // 1. Tracking Line con STEP (bordes duros)
        float trackingPos = fract(vLocalPos.y * 0.4 + uTime * 0.05);
        float trackingBar = 1.0 - step(0.025, abs(trackingPos - 0.5));

        // 2. Base VHS basada en uColor (eliminamos el hardcode de cian)
        float bDist = uIsMouth ? dist : sdQuadraticCircle((pMod + vec2(0.015, 0.0)) / vec2(uWidth, uHeight));
        float bAlpha = 1.0 - smoothstep(-0.02, 0.02, bDist);

        // Mantenemos tu color pero con el sangrado de canal azul para el look analógico
        finalColor = vec3(uColor.r, uColor.g, max(uColor.b, bAlpha * 0.5));

        // 3. Lógica de color de la barra (Mix con blanco 0.3 y brillo 0.3)
        // He usado 0.3 y 0.3 como pediste para asegurar visibilidad
        vec3 barColor = mix(uColor, vec3(1.0), 0.2) * 0.6;

        // Aplicamos la barra y el grano
        finalColor = mix(finalColor, barColor, trackingBar);
        finalColor += grain;
    }

    if (alpha < 0.01) discard;
    FragColor = vec4(finalColor * 1.6, alpha);
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

    // --- CONFIGURACIÓN DE OJOS ---
    // Parámetros: X, Y, Width, Height, Thickness
    float eyeW = 0.35f;
    float eyeH = 0.25f;
    float eyeSpacing = 0.45f;
    float eyeY = 0;
    float thickness = 0.15;


    leftEye_ = new Eye(-eyeSpacing, eyeY, eyeW, eyeH);
    rightEye_ = new Eye(eyeSpacing, eyeY, eyeW, eyeH);

    leftEye_->init();
    rightEye_->init();

    // --- CONFIGURACIÓN DE BOCA ---
    // Parámetros: X, Y, Width, Height
    // Estos valores definen el "origen" y el tamaño de la curva :3
    mouth_ = new Mouth(0.0f, -0.0f, 0.3f, 0.2f);
    mouth_->init();

    aout << "=== Face components initialized successfully ===" << std::endl;
}

void Renderer::renderEyes() {
    if (eyeShaderProgram_ == 0) return;

    // 1. Acceder a las variables globales definidas en main.cpp
    // Estas son las que el JNI modifica desde el menú de Kotlin
    extern bool g_vhsEnabled;
    extern float g_petColor[3];

    // 2. Gestión del tiempo para los Shaders (Efecto de ruido y tracking)
    static float totalTime = 0.0f;
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime_).count();
    lastFrameTime_ = currentTime;
    totalTime += deltaTime;

    // 3. Actualizar lógica de la mascota (parpadeo, mirada, etc.)
    pet_.update(deltaTime);

    // 4. Activar el Shader y pasar los Uniforms del menú
    glUseProgram(eyeShaderProgram_);

    // Pasamos el tiempo total para que las animaciones de ruido no se detengan
    glUniform1f(glGetUniformLocation(eyeShaderProgram_, "uTime"), totalTime);

    // Pasamos el interruptor VHS (0 = Apagado, 1 = Encendido)
    glUniform1i(glGetUniformLocation(eyeShaderProgram_, "uVHSEnabled"), g_vhsEnabled ? 1 : 0);

    // Pasamos el color RGB elegido
    glUniform3f(glGetUniformLocation(eyeShaderProgram_, "uColor"), g_petColor[0], g_petColor[1], g_petColor[2]);

    // 5. Obtener datos de estado de la mascota
    float lookX = pet_.getLookAtX();
    float lookY = pet_.getLookAtY();
    float scale = pet_.getScale();
    float eyeOpenness = pet_.isBlinking() ? 0.1f : 1.0f;

    // 6. Configuración de posiciones
    float eyeSpacing = 0.45f;
    float eyeY = 0.15f;
    float mouthBaseY = -0.1f; // Ajustado para que la boca esté centrada abajo

    // 7. Renderizado de los Ojos
    if (leftEye_ && rightEye_) {
        // Ojo Izquierdo
        leftEye_->setPosition(-eyeSpacing + lookX, eyeY + lookY);
        leftEye_->draw(eyeShaderProgram_, eyeOpenness, scale, g_petColor);

        // Ojo Derecho
        rightEye_->setPosition(eyeSpacing + lookX, eyeY + lookY);
        rightEye_->draw(eyeShaderProgram_, eyeOpenness, scale, g_petColor);
    }

    // 8. Renderizado de la Boca (con un ligero efecto de profundidad parallax)
    if (mouth_) {
        float mouthLookX = lookX * 0.5f;
        float mouthLookY = lookY * 0.5f;
        mouth_->setPosition(0.0f + mouthLookX, mouthBaseY + mouthLookY);
        mouth_->draw(eyeShaderProgram_, g_petColor);
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

        // 1. Obtenemos las coordenadas del toque (primer dedo)
        auto &pointer = motionEvent.pointers[0];
        float rawX = GameActivityPointerAxes_getX(&pointer);
        float rawY = GameActivityPointerAxes_getY(&pointer);

        // 2. Normalizamos las coordenadas al rango [-1.0, 1.0] de OpenGL
        // width_ y height_ son las dimensiones de la superficie de renderizado
        float normX = (2.0f * rawX / (float)width_) - 1.0f;
        float normY = 1.0f - (2.0f * rawY / (float)height_);

        switch (action & AMOTION_EVENT_ACTION_MASK) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_MOVE: {
                // Mientras se mueve o se mantiene presionado
                auto currentTime = std::chrono::high_resolution_clock::now();
                float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime_).count();

                // Actualizamos el punto hacia donde debe mirar
                pet_.setLookAtTarget(normX, normY);
                // Notificamos que el dedo sigue presionado para activar 'isLooking'
                pet_.onHold(deltaTime);
                break;
            }
            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_CANCEL:
                // Al soltar el dedo, la mascota deja de mirar
                pet_.onRelease();
                break;
        }
    }

    // Limpiamos los buffers de entrada para el siguiente frame
    android_app_clear_motion_events(inputBuffer);
    android_app_clear_key_events(inputBuffer);
}