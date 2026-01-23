#ifndef EYE_H
#define EYE_H

#include <GLES3/gl3.h>

enum class EyeShape { CIRCLE, SQUARE, TRIANGLE };

class Eye {
public:
    // Mantenemos los parámetros para que coincidan con Mouth
    Eye(float x, float y, float width, float height, float thickness = 0.15f);
    ~Eye();

    void init();
    void draw(GLuint shaderProgram, float eyeOpenness, float scale);
    void setPosition(float x, float y);

    // Getters para la posición base
    float getX() const { return posX; }
    float getY() const { return posY; }

private:
    GLuint outlineVBO;
    float posX, posY;
    float width, height; // Ahora usamos width y height en lugar de solo radius
    float thickness;
    int vertexCount;
};

#endif