#include "Pet.hpp"
#include <cmath>
#include <algorithm>

Pet::Pet(float initialMood)
        : moodLevel_(50.0f), // Iniciamos en 50 para pruebas
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

    // 2. Lógica de Interpolación de mirada (Tu lógica original)
    float finalTargetX = isBeingHeld_ ? targetLookX_ : 0.0f;
    float finalTargetY = isBeingHeld_ ? targetLookY_ : 0.0f;

    float lerpSpeed = 15.0f;
    currentLookX_ += (finalTargetX - currentLookX_) * lerpSpeed * deltaTime;
    currentLookY_ += (finalTargetY - currentLookY_) * lerpSpeed * deltaTime;
}

void Pet::setLookAtTarget(float x, float y) {
    // Límites proporcionales aplicados al objetivo
    targetLookX_ = std::clamp(x, -1.0f, 1.0f) * 0.20f;
    targetLookY_ = std::clamp(y, -1.0f, 1.0f) * 0.12f;
}

void Pet::onHold(float deltaTime) {
    isBeingHeld_ = true;
    // Aumenta el ánimo mientras acaricias (3 puntos por segundo)
    setMoodLevel(moodLevel_ + (3.0f * deltaTime));
}

void Pet::onRelease() {
    isBeingHeld_ = false; // El update se encargará de volver a (0,0)
}

void Pet::setMoodLevel(float level) {
    moodLevel_ = std::clamp(level, 20.0f, 100.0f); // Mínimo 10, Máximo 100
}

float Pet::getScale() const {
    return 1.0f + std::sin(time_ * 1.5f) * 0.01f;
}