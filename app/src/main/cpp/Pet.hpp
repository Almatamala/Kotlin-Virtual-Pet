#ifndef PET_HPP
#define PET_HPP

#include <string>
#include <algorithm>

enum class PetMood { SAD, ANGRY, NEUTRAL, HAPPY };

class Pet {
public:
    Pet(float initialMood = 50.0f);
    void update(float deltaTime);
    void onTouch();
    void onHold(float deltaTime);
    void setMoodLevel(float level);
    float getMoodLevel() const;
    PetMood getMoodState() const;
    std::string getMoodString() const;

    // Estos son los que alimentan al Renderer:
    bool isBlinking() const;
    float getScale() const;
    float getYOffset() const;

private:
    float moodLevel_;
    float time_;
    float blinkTimer_;
    bool isBlinking_;
    float reactionTimer_;
};

#endif