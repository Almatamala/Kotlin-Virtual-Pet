#ifndef EYE_H
#define EYE_H

#include <GLES3/gl3.h>

enum class EyeShape { };

class Eye {
public:
    // Mantenemos los parámetros para que coincidan con Mouth
    Eye(float x, float y, float width, float height);
    ~Eye();

    void init();
    void draw(GLuint shaderProgram, float eyeOpenness, float scale) const;
    void setPosition(float x, float y);

private:
    GLuint outlineVBO;
    float posX, posY;
    float width, height; // Ahora usamos width y height en lugar de solo radius
};

#endif