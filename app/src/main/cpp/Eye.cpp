#include "Eye.h"

Eye::Eye(float x, float y, float w, float h)
        : posX(x), posY(y), width(w), height(h), outlineVBO(0) {}

Eye::~Eye() { if (outlineVBO) glDeleteBuffers(1, &outlineVBO); }

void Eye::init() {
    // Definimos el lienzo del ojo usando width y height
    float vertices[] = {
            -width, -height,
            width, -height,
            -width,  height,
            width,  height
    };
    glGenBuffers(1, &outlineVBO);
    glBindBuffer(GL_ARRAY_BUFFER, outlineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Eye::draw(GLuint shaderProgram, float eyeOpenness, float scale, const float* color) const {
    glUseProgram(shaderProgram);

    glUniform1i(glGetUniformLocation(shaderProgram, "uIsMouth"), 0); // Modo Ojo
    glUniform2f(glGetUniformLocation(shaderProgram, "uPosition"), posX, posY);
    glUniform1f(glGetUniformLocation(shaderProgram, "uScale"), scale);
    glUniform1f(glGetUniformLocation(shaderProgram, "uEyeOpenness"), eyeOpenness);
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), color[0], color[1], color[2]);
    // Enviamos dimensiones al shader
    glUniform1f(glGetUniformLocation(shaderProgram, "uWidth"), width);
    glUniform1f(glGetUniformLocation(shaderProgram, "uHeight"), height);

    glBindBuffer(GL_ARRAY_BUFFER, outlineVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(0);
}

void Eye::setPosition(float x, float y) { posX = x; posY = y; }