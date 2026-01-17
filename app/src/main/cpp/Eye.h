#ifndef EYE_H
#define EYE_H

#include <GLES3/gl3.h>
#include <vector>

class Eye {
public:
    Eye(float x, float y, float radius);
    ~Eye();

    void init();
    void draw(GLuint shaderProgram, float eyeOpenness, float scale);

    // Setters para animaciones dinámicas
    void setPosition(float x, float y);
    void setPupilOffset(float x, float y);

private:
    std::vector<float> generateCircleVertices(float radius, int segments);

    GLuint whiteVAO, whiteVBO;  // Esclerótica (blanco del ojo)
    GLuint pupilVAO, pupilVBO;  // Pupila (negro)

    float posX, posY;
    float radius;
    float pupilOffsetX, pupilOffsetY;
    int vertexCount;
};

#endif // EYE_H