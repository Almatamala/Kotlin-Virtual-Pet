#ifndef EYE_H
#define EYE_H

#include <GLES3/gl3.h>
#include <vector>
#include <cmath>

enum class EyeShape {
    CIRCLE,
};

class Eye {
public:
    Eye(float x, float y, float radius, EyeShape shape = EyeShape::CIRCLE, float lineThickness = 0.03f);
    ~Eye();

    void init();
    void draw(GLuint shaderProgram, float eyeOpenness, float scale);

    // Setters para animaciones dinámicas
    void setPosition(float x, float y);

private:
    GLuint outlineVBO;

    float posX, posY;
    float radius;
    float pupilOffsetX, pupilOffsetY;
    float lineThickness;
    int vertexCount;
    EyeShape currentShape;
};

#endif // EYE_H