#include <jni.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>
#include "Renderer.h"

// 1. Variables globales (Deben estar aquí para que Renderer.cpp pueda verlas)
bool g_vhsEnabled = false;
float g_petColor[3] = {1.0f, 1.0f, 1.0f};

extern "C" {

// JNI: Cambiar estado VHS
JNIEXPORT void JNICALL
Java_com_example_kotlin_1virtualpet_MainActivity_setVHSEffectNative(JNIEnv* env, jobject obj, jboolean enabled) {
    g_vhsEnabled = enabled;
}

// JNI: Leer estado VHS
JNIEXPORT jboolean JNICALL
Java_com_example_kotlin_1virtualpet_MainActivity_isVHSEnabledNative(JNIEnv* env, jobject obj) {
    return (jboolean)g_vhsEnabled;
}

// JNI: Cambiar Color
JNIEXPORT void JNICALL
Java_com_example_kotlin_1virtualpet_MainActivity_setPetColorNative(JNIEnv* env, jobject obj, jfloat r, jfloat g, jfloat b) {
    g_petColor[0] = r;
    g_petColor[1] = g;
    g_petColor[2] = b;
}

void handle_cmd(android_app *pApp, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            pApp->userData = new Renderer(pApp);
            break;
        case APP_CMD_TERM_WINDOW:
            if (pApp->userData) {
                auto *pRenderer = reinterpret_cast<Renderer *>(pApp->userData);
                pApp->userData = nullptr;
                delete pRenderer;
            }
            break;
    }
}

void android_main(struct android_app *pApp) {
    pApp->onAppCmd = handle_cmd;
    do {
        bool done = false;
        while (!done) {
            int events;
            android_poll_source *pSource;
            int result = ALooper_pollOnce(0, nullptr, &events, (void**)&pSource);
            if (result >= 0) {
                if (pSource) pSource->process(pApp, pSource);
            } else {
                done = true;
            }
        }
        if (pApp->userData) {
            auto *pRenderer = reinterpret_cast<Renderer *>(pApp->userData);
            pRenderer->handleInput();
            pRenderer->render();
        }
    } while (!pApp->destroyRequested);
}
}