#include "Mouth.h"

Mouth::Mouth(float x, float y, float w, float h)
        : posX(x), posY(y), width(w), height(h), vbo(0), vertexCount(4) {
}

Mouth::~Mouth() {
    if (vbo) glDeleteBuffers(1, &vbo);
}

void Mouth::init() {
    // El margen asegura que el lienzo sea lo suficientemente grande para la curva
    float marginW = width * 1.2f;
    float marginH = height * 1.5f;

    float vertices[] = {
            -marginW, -marginH,
            marginW, -marginH,
            -marginW,  marginH,
            marginW,  marginH
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mouth::draw(GLuint shaderProgram) const {
    glUseProgram(shaderProgram);

    glUniform1i(glGetUniformLocation(shaderProgram, "uIsMouth"), 1);
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), 0.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "uThickness"), 0.01f);
    glUniform2f(glGetUniformLocation(shaderProgram, "uPosition"), posX, posY);
    glUniform1f(glGetUniformLocation(shaderProgram, "uScale"), 1.0f);

    // NUEVO: Pasamos las dimensiones del constructor al shader
    glUniform1f(glGetUniformLocation(shaderProgram, "uMouthWidth"), width);
    glUniform1f(glGetUniformLocation(shaderProgram, "uMouthHeight"), height);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mouth::setPosition(float x, float y) {
    posX = x;
    posY = y;
}