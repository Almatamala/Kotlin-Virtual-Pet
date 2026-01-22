#include "Eye.h"
#include <cmath>
#include "AndroidOut.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Eye::Eye(float x, float y, float r, EyeShape shape, float thickness)
        : posX(x), posY(y), radius(r),
          pupilOffsetX(0.0f), pupilOffsetY(0.0f),
          vertexCount(0),
          outlineVBO(0),
          currentShape(shape),
          lineThickness(thickness) {
}

Eye::~Eye() {
    if (outlineVBO) glDeleteBuffers(1, &outlineVBO);
}

void Eye::init() {
    // Definimos el tamaño del "cuadrado" que contiene al ojo usando el radio
    float vertices[] = {
            -radius, -radius,
            radius, -radius,
            -radius,  radius,
            radius,  radius
    };
    vertexCount = 4;

    glGenBuffers(1, &outlineVBO);
    glBindBuffer(GL_ARRAY_BUFFER, outlineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Eye::draw(GLuint shaderProgram, float eyeOpenness, float scale) {
    glUseProgram(shaderProgram);

    GLint posLoc = glGetUniformLocation(shaderProgram, "uPosition");
    GLint scaleLoc = glGetUniformLocation(shaderProgram, "uScale");
    GLint opennessLoc = glGetUniformLocation(shaderProgram, "uEyeOpenness");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
    GLint radiusLoc = glGetUniformLocation(shaderProgram, "uRadius");
    GLint thicknessLoc = glGetUniformLocation(shaderProgram, "uThickness");
    glUniform1f(thicknessLoc, lineThickness);
    glUniform2f(posLoc, posX, posY);
    glUniform1f(scaleLoc, scale);
    glUniform1f(opennessLoc, eyeOpenness);
    glUniform3f(colorLoc, 0.0f, 1.0f, 1.0f);
    glUniform1f(radiusLoc, radius); // <--- Crucial

    glBindBuffer(GL_ARRAY_BUFFER, outlineVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount); // Cambiado a STRIP para el quad

    glDisableVertexAttribArray(0);
}

void Eye::setPosition(float x, float y) {
    posX = x;
    posY = y;
}

