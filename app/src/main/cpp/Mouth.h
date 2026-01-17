#ifndef MOUTH_H
#define MOUTH_H

#include <GLES3/gl3.h>
#include <vector>

class Mouth {
public:
    Mouth(float x, float y, float width, float height);
    ~Mouth();

    void init();
    void draw(GLuint shaderProgram);
    void setPosition(float x, float y);

private:
    std::vector<float> generateWShapeVertices(float width, float height);

    GLuint vbo;
    float posX, posY;
    float width, height;
    int vertexCount;
};

#endif // MOUTH_H