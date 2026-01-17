#include "Eye.h"
#include <cmath>
#include "AndroidOut.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Eye::Eye(float x, float y, float r)
        : posX(x), posY(y), radius(r),
          pupilOffsetX(0.0f), pupilOffsetY(0.0f),
          vertexCount(0),
          whiteVAO(0), whiteVBO(0),
          pupilVAO(0), pupilVBO(0) {
}

Eye::~Eye() {
    if (whiteVAO) glDeleteVertexArrays(1, &whiteVAO);
    if (whiteVBO) glDeleteBuffers(1, &whiteVBO);
    if (pupilVAO) glDeleteVertexArrays(1, &pupilVAO);
    if (pupilVBO) glDeleteBuffers(1, &pupilVBO);
}

std::vector<float> Eye::generateCircleVertices(float r, int segments) {
    std::vector<float> vertices;

    // Centro del círculo
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    // Perímetro del círculo
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        vertices.push_back(r * cos(angle));
        vertices.push_back(r * sin(angle));
    }

    return vertices;
}

void Eye::init() {
    // Generar geometría para el blanco del ojo
    std::vector<float> whiteVertices = generateCircleVertices(radius, 32);
    // Generar geometría para la pupila (40% del tamaño)
    std::vector<float> pupilVertices = generateCircleVertices(radius * 0.4f, 32);

    vertexCount = static_cast<int>(whiteVertices.size() / 2);

    // === Crear VAO/VBO para el blanco del ojo ===
    glGenVertexArrays(1, &whiteVAO);
    glGenBuffers(1, &whiteVBO);

    glBindVertexArray(whiteVAO);
    glBindBuffer(GL_ARRAY_BUFFER, whiteVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 whiteVertices.size() * sizeof(float),
                 whiteVertices.data(),
                 GL_STATIC_DRAW);

    // Atributo de posición (location = 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // === Crear VAO/VBO para la pupila ===
    glGenVertexArrays(1, &pupilVAO);
    glGenBuffers(1, &pupilVBO);

    glBindVertexArray(pupilVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pupilVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 pupilVertices.size() * sizeof(float),
                 pupilVertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    aout << "Eye initialized at (" << posX << ", " << posY << ")" << std::endl;
}

void Eye::draw(GLuint shaderProgram, float eyeOpenness, float scale) {
    glUseProgram(shaderProgram);

    // Obtener ubicaciones de uniforms
    GLint posLoc = glGetUniformLocation(shaderProgram, "uPosition");
    GLint scaleLoc = glGetUniformLocation(shaderProgram, "uScale");
    GLint opennessLoc = glGetUniformLocation(shaderProgram, "uEyeOpenness");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");

    // === Dibujar blanco del ojo (esclerótica) ===
    glUniform2f(posLoc, posX, posY);
    glUniform1f(scaleLoc, scale);
    glUniform1f(opennessLoc, eyeOpenness);
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f); // Color blanco

    glBindVertexArray(whiteVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);

    // === Dibujar pupila ===
    // La pupila se dibuja con un offset y puede moverse dentro del ojo
    glUniform2f(posLoc, posX + pupilOffsetX * scale, posY + pupilOffsetY * scale);
    glUniform3f(colorLoc, 0.1f, 0.1f, 0.15f); // Negro azulado

    glBindVertexArray(pupilVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);

    glBindVertexArray(0);
}

void Eye::setPosition(float x, float y) {
    posX = x;
    posY = y;
}

void Eye::setPupilOffset(float x, float y) {
    // Limitar el movimiento de la pupila para que no se salga del ojo
    float maxOffset = radius * 0.3f;
    float distance = sqrt(x * x + y * y);

    if (distance > maxOffset) {
        pupilOffsetX = (x / distance) * maxOffset;
        pupilOffsetY = (y / distance) * maxOffset;
    } else {
        pupilOffsetX = x;
        pupilOffsetY = y;
    }
}