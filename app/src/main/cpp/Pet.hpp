#ifndef PET_HPP
#define PET_HPP

#include <algorithm>

enum class PetMood { SAD, ANGRY, NEUTRAL, HAPPY };

class Pet {
public:
    Pet(float initialMood = 50.0f);
    void update(float deltaTime);

    // Gestión de entrada
    void onHold(float deltaTime);
    void onRelease();
    void setLookAtTarget(float x, float y);

    // Getters de mirada (devuelven la posición ya suavizada y limitada)
    float getLookAtX() const { return currentLookX_; }
    float getLookAtY() const { return currentLookY_; }

    bool isBlinking() const { return isBlinking_; }
    float getScale() const;

    void setMoodLevel(float level);
    PetMood getMoodState() const;

private:
    float moodLevel_;
    float time_;
    float blinkTimer_;
    bool isBlinking_;
    bool isBeingHeld_;

    // Variables para el suavizado y los límites proporcionales
    float targetLookX_, targetLookY_;
    float currentLookX_, currentLookY_;
};

#endif