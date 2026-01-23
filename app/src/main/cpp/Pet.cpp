#include "Pet.hpp"
#include <cmath>

Pet::Pet(float initialMood)
        : moodLevel_(initialMood),
          time_(0.0f),
          blinkTimer_(0.0f),
          isBlinking_(false),
          isBeingHeld_(false),
          targetLookX_(0.0f), targetLookY_(0.0f),
          currentLookX_(0.0f), currentLookY_(0.0f) {}

void Pet::update(float deltaTime) {
    time_ += deltaTime;

    // 1. Ciclo de parpadeo
    blinkTimer_ += deltaTime;
    if (!isBlinking_ && blinkTimer_ > 5.0f) {
        isBlinking_ = true;
        blinkTimer_ = 0.0f;
    }
    if (isBlinking_ && blinkTimer_ > 0.15f) {
        isBlinking_ = false;
        blinkTimer_ = 0.0f;
    }

    // 2. Lógica de Interpolación (Suavizado)
    // Si soltamos la pantalla, el objetivo vuelve a ser el centro (0,0)
    float finalTargetX = isBeingHeld_ ? targetLookX_ : 0.0f;
    float finalTargetY = isBeingHeld_ ? targetLookY_ : 0.0f;

    // Velocidad a la que los ojos siguen al dedo o regresan al centro
    float lerpSpeed = 15.0f; // Aumentado de 10.0f para un retorno más rápido
    currentLookX_ += (finalTargetX - currentLookX_) * lerpSpeed * deltaTime;
    currentLookY_ += (finalTargetY - currentLookY_) * lerpSpeed * deltaTime;
}

void Pet::setLookAtTarget(float x, float y) {
    // EL LÍMITE ES PROPORCIONAL:
    // Multiplicamos la posición del dedo (-1 a 1) por el rango máximo de movimiento.
    // Cuanto más lejos esté el dedo (x cercano a 1), mayor será el desplazamiento (0.20f).
    targetLookX_ = std::clamp(x, -1.0f, 1.0f) * 0.20f;
    targetLookY_ = std::clamp(y, -1.0f, 1.0f) * 0.12f;
}

void Pet::onHold(float deltaTime) {
    isBeingHeld_ = true; // Activa la mirada y la interacción
    setMoodLevel(moodLevel_ + (10.0f * deltaTime));
}

void Pet::onRelease() {
    isBeingHeld_ = false; // Al desactivarse, el update() hará que regrese a (0,0)
}

void Pet::setMoodLevel(float level) {
    moodLevel_ = std::clamp(level, 0.0f, 100.0f);
}

PetMood Pet::getMoodState() const {
    if (moodLevel_ < 25.0f) return PetMood::SAD;
    if (moodLevel_ < 50.0f) return PetMood::ANGRY;
    if (moodLevel_ < 75.0f) return PetMood::NEUTRAL;
    return PetMood::HAPPY;
}

float Pet::getScale() const {
    return 1.0f + std::sin(time_ * 1.5f) * 0.01f;
}