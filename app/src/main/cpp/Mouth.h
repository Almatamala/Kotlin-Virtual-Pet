#ifndef MOUTH_H
#define MOUTH_H

#include <GLES3/gl3.h>
#include <vector>

class Mouth {
public:
    Mouth(float x, float y, float width, float height);
    ~Mouth();

    void init();
    void draw(GLuint shaderProgram, const float* color) const;
    void setPosition(float x, float y);
    float getX() const { return posX; }
    float getY() const { return posY; }

private:
    GLuint vbo;
    float posX, posY;
    float width, height;
    int vertexCount;
};

#endif // MOUTH_H