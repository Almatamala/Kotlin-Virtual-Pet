#include <jni.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>
#include <thread>
#include <chrono>
#include "Renderer.h"
#include "AndroidOut.h"

// VARIABLES GLOBALES DE ESTADO
bool g_vhsEnabled = false;
float g_petColor[3] = {1.0f, 1.0f, 1.0f};
struct android_app* g_app_ptr = nullptr;

// Variables de estado del ciclo de vida
static bool g_has_window = false;
static bool g_has_focus = false;
static bool g_is_resumed = false;

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

JNIEXPORT jfloat JNICALL
Java_com_example_kotlin_1virtualpet_MainActivity_getPetMoodNative(JNIEnv* env, jobject obj) {
    if (g_app_ptr && g_app_ptr->userData) {
        auto* pRenderer = reinterpret_cast<Renderer*>(g_app_ptr->userData);
        return pRenderer->getPet().getMoodLevel();
    }
    return 50.0f;
}

// Función para verificar si debemos crear el Renderer
bool should_create_renderer(android_app *pApp) {
    bool ready = g_has_window && g_has_focus && g_is_resumed && (pApp->window != nullptr);
    if (!ready) {
        aout << "Not ready to create renderer - window:" << g_has_window
             << " focus:" << g_has_focus
             << " resumed:" << g_is_resumed
             << " window_ptr:" << (pApp->window != nullptr) << std::endl;
    }
    return ready;
}

// Función para intentar crear el Renderer si está listo
void try_create_renderer(android_app *pApp) {
    if (pApp->userData == nullptr && should_create_renderer(pApp)) {
        aout << "=== All conditions met, waiting for GL context to stabilize ===" << std::endl;

        // Delay corto para GL (la pantalla de carga en Kotlin maneja el UX)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        aout << "=== Creating Renderer ===" << std::endl;
        pApp->userData = new Renderer(pApp);
    }
}

void handle_cmd(android_app *pApp, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            aout << "=== APP_CMD_INIT_WINDOW ===" << std::endl;
            g_has_window = true;
            // NO crear Renderer aquí - esperar a tener focus también
            try_create_renderer(pApp);
            break;

        case APP_CMD_TERM_WINDOW:
            aout << "=== APP_CMD_TERM_WINDOW ===" << std::endl;
            g_has_window = false;
            if (pApp->userData) {
                aout << "Destroying Renderer" << std::endl;
                auto *pRenderer = reinterpret_cast<Renderer *>(pApp->userData);
                delete pRenderer;
                pApp->userData = nullptr;
            }
            break;

        case APP_CMD_GAINED_FOCUS:
            aout << "=== APP_CMD_GAINED_FOCUS ===" << std::endl;
            g_has_focus = true;
            try_create_renderer(pApp);
            break;

        case APP_CMD_LOST_FOCUS:
            aout << "=== APP_CMD_LOST_FOCUS ===" << std::endl;
            g_has_focus = false;
            break;

        case APP_CMD_PAUSE:
            aout << "=== APP_CMD_PAUSE ===" << std::endl;
            g_is_resumed = false;
            break;

        case APP_CMD_RESUME:
            aout << "=== APP_CMD_RESUME ===" << std::endl;
            g_is_resumed = true;
            try_create_renderer(pApp);
            break;

        case APP_CMD_STOP:
            aout << "=== APP_CMD_STOP ===" << std::endl;
            break;

        case APP_CMD_START:
            aout << "=== APP_CMD_START ===" << std::endl;
            break;

        case APP_CMD_WINDOW_RESIZED:
            aout << "=== APP_CMD_WINDOW_RESIZED ===" << std::endl;
            break;

        case APP_CMD_WINDOW_REDRAW_NEEDED:
            aout << "=== APP_CMD_WINDOW_REDRAW_NEEDED ===" << std::endl;
            break;

        case APP_CMD_DESTROY:
            aout << "=== APP_CMD_DESTROY ===" << std::endl;
            break;
    }
}

void android_main(struct android_app *pApp) {
    aout << "=== android_main starting ===" << std::endl;

    g_app_ptr = pApp;
    pApp->onAppCmd = handle_cmd;

    int frameCount = 0;

    do {
        // Procesar todos los eventos pendientes
        int events;
        android_poll_source *pSource;

        // Usar timeout de 0 cuando hay renderer activo, -1 cuando está pausado
        int timeout = (pApp->userData != nullptr) ? 0 : -1;

        while (true) {
            int result = ALooper_pollOnce(timeout, nullptr, &events, (void**)&pSource);

            if (result == ALOOPER_POLL_TIMEOUT) {
                break;
            }

            if (result == ALOOPER_POLL_ERROR) {
                aout << "ERROR: ALooper_pollOnce returned error" << std::endl;
                break;
            }

            if (pSource != nullptr) {
                pSource->process(pApp, pSource);
            }

            // Si no hay renderer, seguir esperando eventos
            if (pApp->userData == nullptr) {
                continue;
            } else {
                break;
            }
        }

        // Renderizar frame solo si tenemos renderer activo
        if (pApp->userData != nullptr && pApp->window != nullptr) {
            auto *pRenderer = reinterpret_cast<Renderer *>(pApp->userData);
            pRenderer->handleInput();
            pRenderer->render();

            frameCount++;
            if (frameCount % 300 == 0) {
                aout << "Rendered " << frameCount << " frames" << std::endl;
            }
        }

    } while (!pApp->destroyRequested);

    aout << "=== android_main exiting ===" << std::endl;

    // Limpiar el renderer si todavía existe
    if (pApp->userData) {
        aout << "Final cleanup of Renderer" << std::endl;
        auto *pRenderer = reinterpret_cast<Renderer *>(pApp->userData);
        delete pRenderer;
        pApp->userData = nullptr;
    }
}

} // extern "C"