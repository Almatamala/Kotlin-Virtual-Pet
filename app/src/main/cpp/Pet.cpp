#include "Pet.hpp"
#include <algorithm>
#include <cmath>

// Constructor
Pet::Pet(float initialMood)
        : moodLevel_(initialMood),
          time_(0.0f),
          blinkTimer_(0.0f),
          isBlinking_(false),
          reactionTimer_(0.0f) {}

void Pet::update(float deltaTime) {
    time_ += deltaTime;

    // 1. Lógica de Parpadeo
    blinkTimer_ += deltaTime;
    if (!isBlinking_ && blinkTimer_ > 3.0f) {
        isBlinking_ = true;
        blinkTimer_ = 0.0f;
    }
    if (isBlinking_ && blinkTimer_ > 0.15f) {
        isBlinking_ = false;
        blinkTimer_ = 0.0f;
    }

    // 2. Temporizador de reacción
    if (reactionTimer_ > 0.0f) {
        reactionTimer_ -= deltaTime;
    }
}

void Pet::onTouch() {
    reactionTimer_ = 0.5f;
}

void Pet::onHold(float deltaTime) {
    setMoodLevel(moodLevel_ + (15.0f * deltaTime));
}

void Pet::setMoodLevel(float level) {
    // Si std::clamp te da error, asegúrate de compilar con C++17 en CMake
    moodLevel_ = std::clamp(level, 0.0f, 100.0f);
}

float Pet::getMoodLevel() const { return moodLevel_; }

PetMood Pet::getMoodState() const {
    if (moodLevel_ < 25.0f) return PetMood::SAD;
    if (moodLevel_ < 50.0f) return PetMood::ANGRY;
    if (moodLevel_ < 75.0f) return PetMood::NEUTRAL;
    return PetMood::HAPPY;
}

bool Pet::isBlinking() const { return isBlinking_; }

float Pet::getScale() const {
    float scale = 1.0f + std::sin(time_ * 2.0f) * 0.05f;
    if (reactionTimer_ > 0.0f && getMoodState() == PetMood::ANGRY) {
        scale *= 1.2f;
    }
    return scale;
}

float Pet::getYOffset() const {
    if (getMoodState() == PetMood::HAPPY) {
        return std::abs(std::sin(time_ * 5.0f)) * 0.2f;
    }
    if (reactionTimer_ > 0.0f) {
        return 0.1f;
    }
    return 0.0f;
}

std::string Pet::getMoodString() const {
    switch (getMoodState()) {
        case PetMood::SAD:     return "Triste";
        case PetMood::ANGRY:   return "Enojado";
        case PetMood::NEUTRAL: return "Neutral";
        case PetMood::HAPPY:   return "Feliz";
        default:               return "Desconocido";
    }
}