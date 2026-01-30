#ifndef PET_HPP
#define PET_HPP

#include <algorithm>

enum class PetMood { SAD, ANGRY, NEUTRAL, HAPPY };

class Pet {
public:
    Pet(float initialMood = 50.0f);
    void update(float deltaTime);

    void onHold(float deltaTime);
    void onRelease();
    void setLookAtTarget(float x, float y);

    float getLookAtX() const { return currentLookX_; }
    float getLookAtY() const { return currentLookY_; }

    bool isBlinking() const { return isBlinking_; }
    float getScale() const;

    void setMoodLevel(float level);
    float getMoodLevel() const { return moodLevel_; }

private:
    float moodLevel_;
    float time_;
    float blinkTimer_;
    bool isBlinking_;
    bool isBeingHeld_;

    float targetLookX_, targetLookY_;
    float currentLookX_, currentLookY_;
};

#endif