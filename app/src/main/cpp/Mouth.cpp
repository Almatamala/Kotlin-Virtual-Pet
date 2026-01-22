#include "Mouth.h"
#include "AndroidOut.h"

Mouth::Mouth(float x, float y, float w, float h)
        : posX(x), posY(y), width(w), height(h), vbo(0), vertexCount(0) {
}

Mouth::~Mouth() {
    if (vbo) glDeleteBuffers(1, &vbo);
}

std::vector<float> Mouth::generateWShapeVertices(float w, float h) {
    std::vector<float> vertices;

    // Forma de W (zigzag)
    float halfW = w / 2.0f;

    // Puntos de la W de izquierda a derecha
    vertices.push_back(-halfW);         vertices.push_back(h);      // Punto 1: arriba izquierda
    vertices.push_back(-halfW * 0.5f);  vertices.push_back(0.0f);   // Punto 2: valle izquierdo
    vertices.push_back(0.0f);           vertices.push_back(h);      // Punto 3: pico central
    vertices.push_back(halfW * 0.5f);   vertices.push_back(0.0f);   // Punto 4: valle derecho
    vertices.push_back(halfW);          vertices.push_back(h);      // Punto 5: arriba derecha

    return vertices;
}

void Mouth::init() {
    std::vector<float> vertices = generateWShapeVertices(width, height);
    vertexCount = static_cast<int>(vertices.size() / 2);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(float),
                 vertices.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    aout << "Mouth initialized at (" << posX << ", " << posY << ")" << std::endl;
}

void Mouth::draw(GLuint shaderProgram) {
    glUseProgram(shaderProgram);

    GLint posLoc = glGetUniformLocation(shaderProgram, "uPosition");
    GLint scaleLoc = glGetUniformLocation(shaderProgram, "uScale");
    GLint opennessLoc = glGetUniformLocation(shaderProgram, "uEyeOpenness");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");

    // NUEVOS UNIFORMS: Para que el shader no descarte los píxeles de la boca
    GLint radiusLoc = glGetUniformLocation(shaderProgram, "uRadius");
    GLint thicknessLoc = glGetUniformLocation(shaderProgram, "uThickness");

    glUniform2f(posLoc, posX, posY);
    glUniform1f(scaleLoc, 1.0f);
    glUniform1f(opennessLoc, 1.0f);
    glUniform3f(colorLoc, 0.0f, 1.0f, 1.0f); // Cyan

    // Configuramos valores que "desactivan" el efecto de círculo
    glUniform1f(radiusLoc, 100.0f);    // Radio gigante para que dist sea casi 0
    glUniform1f(thicknessLoc, 1.0f);   // Grosor total para que no haya hueco central

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0); // Usamos 0 explícitamente como en los ojos
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glLineWidth(12.0f);
    glDrawArrays(GL_LINE_STRIP, 0, vertexCount);

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mouth::setPosition(float x, float y) {
    posX = x;
    posY = y;
}