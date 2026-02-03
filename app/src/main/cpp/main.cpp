#include <jni.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>
#include "Renderer.h"

// VARIABLES GLOBALES DE ESTADO
bool g_vhsEnabled = false;
float g_petColor[3] = {1.0f, 1.0f, 1.0f};
struct android_app* g_app_ptr = nullptr;

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_kotlin_1virtualpet_MainActivity_setVHSEffectNative(JNIEnv* env, jobject obj, jboolean enabled) {
    g_vhsEnabled = enabled;
}

JNIEXPORT jboolean JNICALL
Java_com_example_kotlin_1virtualpet_MainActivity_isVHSEnabledNative(JNIEnv* env, jobject obj) {
    return (jboolean)g_vhsEnabled;
}

JNIEXPORT void JNICALL
Java_com_example_kotlin_1virtualpet_MainActivity_setPetColorNative(JNIEnv* env, jobject obj, jfloat r, jfloat g, jfloat b) {
    g_petColor[0] = r;
    g_petColor[1] = g;
    g_petColor[2] = b;
}

JNIEXPORT void JNICALL
Java_com_example_kotlin_1virtualpet_MainActivity_setPetMoodNative(JNIEnv* env, jobject obj, jfloat mood) {
    if (g_app_ptr && g_app_ptr->userData) {
        auto* pRenderer = reinterpret_cast<Renderer*>(g_app_ptr->userData);
        pRenderer->getPet().setMoodLevel(mood);
    }
}

// Función para que Kotlin obtenga el ánimo actual (por si subió al acariciar)
JNIEXPORT jfloat JNICALL
Java_com_example_kotlin_1virtualpet_MainActivity_getPetMoodNative(JNIEnv* env, jobject obj) {
    if (g_app_ptr && g_app_ptr->userData) {
        auto* pRenderer = reinterpret_cast<Renderer*>(g_app_ptr->userData);
        return pRenderer->getPet().getMoodLevel();
    }
    return 50.0f;
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
    g_app_ptr = pApp; // Guardamos el puntero global
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