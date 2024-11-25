#ifndef PROJECTMESH_H
#define PROJECTMESH_H

#include <vector>
#include "point3.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

struct Vertex{
    point3d p;
    Vertex() {}
    Vertex(double x , double y , double z) : p(x,y,z) {}
    double & operator [] (unsigned int c) { return p[c]; }
    double operator [] (unsigned int c) const { return p[c]; }
};
struct Triangle{
    unsigned int corners[3];
    unsigned int & operator [] (unsigned int c) { return corners[c]; }
    unsigned int operator [] (unsigned int c) const { return corners[c]; }
    unsigned int size() const { return 3 ; }
};
struct Mesh{
    std::vector< Vertex > vertices;
    std::vector< Triangle > triangles;
    QOpenGLVertexArrayObject *vao = nullptr;
    QOpenGLBuffer *vbo_vertex = nullptr;
    QOpenGLBuffer *vbo_triangles = nullptr;

    void initVAO(QOpenGLShaderProgram* program) {

        qDebug() << vertices.size();
        qDebug() << triangles.size();

        if (vertices.empty() || triangles.empty()){
            qDebug() << "No vertices or triangles!";
            return;
        }

        program->bind();

        vao = new QOpenGLVertexArrayObject();
        if (!vao->create()){
            program->release();
            qDebug() << "Could not create VAO!";
            return;
        }
        vao->bind();

        vbo_vertex = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        if (!vbo_vertex->create()){
            qDebug() << "Could not create vertex VBO!";
            program->release();
            return;
        }
        vbo_vertex->bind();
        vbo_vertex->setUsagePattern(QOpenGLBuffer::StaticDraw);
        vbo_vertex->allocate(vertices.data(), vertices.size() * sizeof(point3d));

        vbo_triangles = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        if (!vbo_triangles->create()){
            qDebug() << "Could not create triangles VBO!";
            program->release();
            return;
        }
        vbo_triangles->bind();
        vbo_triangles->setUsagePattern(QOpenGLBuffer::StaticDraw);
        vbo_triangles->allocate(triangles.data(), triangles.size() * 3 * sizeof(uint));

        program->enableAttributeArray(0);
        program->setAttributeBuffer(0, GL_DOUBLE, offsetof(Vertex, p), 3, sizeof(Vertex));

        vao->release();
        program->release();
    }

    void render(QOpenGLShaderProgram* program) {
        program->bind();
        if (vao == nullptr){
            return;
        }
        vao->bind();

        glDrawElements(GL_TRIANGLES, triangles.size() * 3, GL_UNSIGNED_INT, nullptr);

        vao->release();
        program->release();
    }

    void clear() {
        vao->destroy();
        vbo_vertex->destroy();
        vbo_triangles->destroy();
    }
};



#endif // PROJECTMESH_H
