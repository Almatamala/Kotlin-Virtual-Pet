#include <string>
#include <algorithm>

enum class PetMood { SAD, ANGRY, NEUTRAL, HAPPY };

class Pet {
public:
    Pet(float initialMood = 50.0f);
    void update(float deltaTime);
    void onTouch();
    void onHold(float deltaTime);
    void onRelease(); // <--- Implementado
    void setMoodLevel(float level);
    PetMood getMoodState() const;

    bool isBlinking() const;
    float getScale() const;
    float getYOffset() const;

private:
    float moodLevel_;
    float time_;
    float blinkTimer_;
    bool isBlinking_;
    float reactionTimer_;
    bool isBeingHeld_; // Nueva bandera útil para estados de ánimo
};
