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

std::vector<float> Eye::generateThickOutline(const std::vector<float>& centerLine, float thickness) {
    std::vector<float> vertices;

    // centerLine tiene: centro (x,y), luego los puntos del perímetro
    // Saltar el primer punto (centro) y procesar el resto
    int numPoints = (centerLine.size() / 2) - 1; // -1 para excluir el centro

    for (int i = 0; i < numPoints; i++) {
        // Índices en el array (recordar que el primer par es el centro)
        int idx1 = (i + 1) * 2;
        int idx2 = ((i + 1) % numPoints + 1) * 2;

        float x1 = centerLine[idx1];
        float y1 = centerLine[idx1 + 1];
        float x2 = centerLine[idx2];
        float y2 = centerLine[idx2 + 1];

        // Calcular vector perpendicular (normal)
        float dx = x2 - x1;
        float dy = y2 - y1;
        float len = sqrt(dx * dx + dy * dy);

        if (len > 0.0001f) {
            // Normal unitario
            float nx = -dy / len;
            float ny = dx / len;

            // Crear un rectángulo con grosor
            float halfThick = thickness / 2.0f;

            // Punto exterior 1
            vertices.push_back(x1 + nx * halfThick);
            vertices.push_back(y1 + ny * halfThick);

            // Punto interior 1
            vertices.push_back(x1 - nx * halfThick);
            vertices.push_back(y1 - ny * halfThick);

            // Punto interior 2
            vertices.push_back(x2 - nx * halfThick);
            vertices.push_back(y2 - ny * halfThick);

            // Punto exterior 1 (repetir para segundo triángulo)
            vertices.push_back(x1 + nx * halfThick);
            vertices.push_back(y1 + ny * halfThick);

            // Punto interior 2 (repetir)
            vertices.push_back(x2 - nx * halfThick);
            vertices.push_back(y2 - ny * halfThick);

            // Punto exterior 2
            vertices.push_back(x2 + nx * halfThick);
            vertices.push_back(y2 + ny * halfThick);
        }
    }

    return vertices;
}

std::vector<float> Eye::generateCircleVertices(float r, int segments) {
    std::vector<float> vertices;

    // Centro
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    // Perímetro
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        vertices.push_back(r * cos(angle));
        vertices.push_back(r * sin(angle));
    }

    return vertices;
}

std::vector<float> Eye::generatePolygonVertices(float r, int sides) {
    std::vector<float> vertices;

    // Centro
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    // Crear polígono regular
    // Rotar -90 grados para que un lado quede arriba
    float angleOffset = -M_PI / 2.0f;

    for (int i = 0; i <= sides; i++) {
        float angle = angleOffset + (2.0f * M_PI * i / sides);
        vertices.push_back(r * cos(angle));
        vertices.push_back(r * sin(angle));
    }

    return vertices;
}

std::vector<float> Eye::generateRectangleVertices(float width, float height) {
    std::vector<float> vertices;

    // Centro
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    float halfW = width / 2.0f;
    float halfH = height / 2.0f;

    // Crear rectángulo (4 esquinas + cerrar)
    vertices.push_back(halfW);   vertices.push_back(halfH);   // Superior derecha
    vertices.push_back(-halfW);  vertices.push_back(halfH);   // Superior izquierda
    vertices.push_back(-halfW);  vertices.push_back(-halfH);  // Inferior izquierda
    vertices.push_back(halfW);   vertices.push_back(-halfH);  // Inferior derecha
    vertices.push_back(halfW);   vertices.push_back(halfH);   // Cerrar

    return vertices;
}

std::vector<float> Eye::generateCapsuleVertices(float width, float height) {
    std::vector<float> vertices;

    // Centro
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;
    float capRadius = halfHeight;

    int segments = 16;

    // Semicírculo derecho (de arriba a abajo)
    for (int i = 0; i <= segments / 2; i++) {
        float angle = -M_PI / 2.0f + (M_PI * i) / (segments / 2);
        vertices.push_back((halfWidth - capRadius) + capRadius * cos(angle));
        vertices.push_back(capRadius * sin(angle));
    }

    // Semicírculo izquierdo (de abajo a arriba)
    for (int i = 0; i <= segments / 2; i++) {
        float angle = M_PI / 2.0f + (M_PI * i) / (segments / 2);
        vertices.push_back(-(halfWidth - capRadius) + capRadius * cos(angle));
        vertices.push_back(capRadius * sin(angle));
    }

    return vertices;
}

std::vector<float> Eye::generateShapeVertices(EyeShape shape, float r) {
    switch(shape) {
        case EyeShape::CIRCLE:
            return generateCircleVertices(r, 32);

        case EyeShape::HEXAGON:
            return generatePolygonVertices(r, 6);

        case EyeShape::OCTAGON:
            return generatePolygonVertices(r, 8);

        case EyeShape::SQUARE:
            return generatePolygonVertices(r, 4);

        case EyeShape::TRIANGLE:
            return generatePolygonVertices(r, 3);

        case EyeShape::RECTANGLE:
            return generateRectangleVertices(r * 1.8f, r * 1.0f);

        case EyeShape::CAPSULE:
            return generateCapsuleVertices(r * 1.8f, r * 1.0f);

        default:
            return generateCircleVertices(r, 32);
    }
}

void Eye::init() {
    // Generar línea central según la forma seleccionada
    std::vector<float> centerLine = generateShapeVertices(currentShape, radius);

    // Generar contorno grueso a partir de la línea central
    std::vector<float> outlineVertices = generateThickOutline(centerLine, lineThickness);

    vertexCount = static_cast<int>(outlineVertices.size() / 2);

    // Crear VBO para el contorno grueso
    glGenBuffers(1, &outlineVBO);
    glBindBuffer(GL_ARRAY_BUFFER, outlineVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 outlineVertices.size() * sizeof(float),
                 outlineVertices.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    const char* shapeName;
    switch(currentShape) {
        case EyeShape::CIRCLE:    shapeName = "CIRCLE"; break;
        case EyeShape::HEXAGON:   shapeName = "HEXAGON"; break;
        case EyeShape::OCTAGON:   shapeName = "OCTAGON"; break;
        case EyeShape::SQUARE:    shapeName = "SQUARE"; break;
        case EyeShape::TRIANGLE:  shapeName = "TRIANGLE"; break;
        case EyeShape::RECTANGLE: shapeName = "RECTANGLE"; break;
        case EyeShape::CAPSULE:   shapeName = "CAPSULE"; break;
        default:                  shapeName = "UNKNOWN"; break;
    }

    aout << "Eye initialized at (" << posX << ", " << posY
         << ") with shape " << shapeName
         << ", thickness " << lineThickness
         << " (" << vertexCount << " vertices) - NO PUPILS" << std::endl;
}

void Eye::draw(GLuint shaderProgram, float eyeOpenness, float scale) {
    glUseProgram(shaderProgram);

    // Obtener ubicaciones de uniforms
    GLint posLoc = glGetUniformLocation(shaderProgram, "uPosition");
    GLint scaleLoc = glGetUniformLocation(shaderProgram, "uScale");
    GLint opennessLoc = glGetUniformLocation(shaderProgram, "uEyeOpenness");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
    GLint posAttrib = 0;

    // Dibujar el contorno grueso como triángulos
    glUniform2f(posLoc, posX, posY);
    glUniform1f(scaleLoc, scale);
    glUniform1f(opennessLoc, eyeOpenness);
    glUniform3f(colorLoc, 0.0f, 1.0f, 1.0f); // Cyan

    glBindBuffer(GL_ARRAY_BUFFER, outlineVBO);
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Dibujar como triángulos (no necesita glLineWidth)
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    glDisableVertexAttribArray(posAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Eye::setPosition(float x, float y) {
    posX = x;
    posY = y;
}

void Eye::setPupilOffset(float x, float y) {
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

void Eye::setShape(EyeShape shape) {
    if (currentShape != shape) {
        currentShape = shape;
        // Nota: necesitarías llamar a init() de nuevo para regenerar la geometría
    }
}