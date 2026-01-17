#ifndef EYE_H
#define EYE_H

#include <GLES3/gl3.h>
#include <vector>
#include <cmath>

enum class EyeShape {
    CIRCLE,      // Círculo suave (muchos segmentos)
    HEXAGON,     // 6 lados
    OCTAGON,     // 8 lados
    SQUARE,      // 4 lados
    TRIANGLE,    // 3 lados
    RECTANGLE,   // Rectángulo horizontal
    CAPSULE      // Forma de píldora (estilo EVA)
};

class Eye {
public:
    Eye(float x, float y, float radius, EyeShape shape = EyeShape::CIRCLE, float lineThickness = 0.03f);
    ~Eye();

    void init();
    void draw(GLuint shaderProgram, float eyeOpenness, float scale);

    // Setters para animaciones dinámicas
    void setPosition(float x, float y);
    void setPupilOffset(float x, float y);
    void setShape(EyeShape shape);
    void setLineThickness(float thickness);

private:
    std::vector<float> generateShapeVertices(EyeShape shape, float radius);
    std::vector<float> generateCircleVertices(float radius, int segments);
    std::vector<float> generatePolygonVertices(float radius, int sides);
    std::vector<float> generateRectangleVertices(float width, float height);
    std::vector<float> generateCapsuleVertices(float width, float height);
    std::vector<float> generateThickOutline(const std::vector<float>& centerLine, float thickness);

    GLuint outlineVBO;

    float posX, posY;
    float radius;
    float pupilOffsetX, pupilOffsetY;
    float lineThickness;
    int vertexCount;
    EyeShape currentShape;
};

#endif // EYE_H