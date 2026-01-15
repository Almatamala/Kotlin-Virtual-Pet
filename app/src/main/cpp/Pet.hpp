#ifndef PET_HPP
#define PET_HPP

#include <string>   // Necesario para std::string
#include <algorithm> // Necesario para std::clamp

enum class PetMood { SAD, ANGRY, NEUTRAL, HAPPY };

class Pet {
public:
    // El constructor debe coincidir con el del .cpp
    Pet(float initialMood = 50.0f);

    void update(float deltaTime);
    void onTouch();
    void onHold(float deltaTime);

    void setMoodLevel(float level);
    float getMoodLevel() const;
    PetMood getMoodState() const;
    std::string getMoodString() const;

    // Métodos para el Renderer (asegúrate de que existan aquí)
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
