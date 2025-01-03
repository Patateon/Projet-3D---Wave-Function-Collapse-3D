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
    std::vector< Vertex > normales;
    std::vector< QVector2D > uv;
    std::vector< Triangle > triangles;
    QOpenGLVertexArrayObject *vao = nullptr;
    QOpenGLBuffer *vbo_vertex = nullptr;
    QOpenGLBuffer *vbo_normales = nullptr;
    QOpenGLBuffer *vbo_uv = nullptr;
    QOpenGLBuffer *vbo_triangles = nullptr;

    // Normal to draw
    QOpenGLVertexArrayObject *vao_line_normal = nullptr;
    QOpenGLBuffer *vbo_normal_line = nullptr;
    std::vector< Vertex > lineVertices;

    void initVAONormalLine(QOpenGLShaderProgram* program, float scale) {
        if (vertices.size() == 0 ||
                vertices.size() != normales.size()){
            qWarning() << "Invalid vertices and normales sizes";
        }

        lineVertices.clear();
        for(uint i = 0; i < vertices.size(); i++){
            point3d start = vertices[i].p;
            point3d end = vertices[i].p + normales[i].p * scale;
            Vertex vstart, vend;
            vstart.p = start;
            vend.p = end;

            lineVertices.push_back(vstart);
            lineVertices.push_back(vend);
        }

        vao_line_normal = new QOpenGLVertexArrayObject();
        if (!vao_line_normal->create()){
            program->release();
            qWarning() << "Could not create line VAO!";
            return;
        }
        vao_line_normal->bind();

        vbo_normal_line = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        if (!vbo_normal_line->create()){
            qWarning() << "Could not create vertex VBO!";
            program->release();
            return;
        }
        vbo_normal_line->bind();
        vbo_normal_line->setUsagePattern(QOpenGLBuffer::StaticDraw);
        vbo_normal_line->allocate(
                    lineVertices.data(), lineVertices.size() * sizeof(point3d));

        program->enableAttributeArray(0);
        program->setAttributeBuffer(
                    0, GL_DOUBLE, offsetof(Vertex, p), 3, sizeof(Vertex));

        vao_line_normal->release();
        program->release();
    }

    void renderVAONormalLine(QOpenGLShaderProgram* program){

        if (vao_line_normal == nullptr){
            return;
        }
        vao_line_normal->bind();

        glDrawArrays(GL_LINES, 0, lineVertices.size());

        vao_line_normal->release();
        program->release();
    }

    void computeNormales(){
        normales.clear();
        normales.resize(vertices.size(), Vertex(0.0, 0.0, 0.0));

        for(unsigned int tIt = 0; tIt < triangles.size(); tIt+=3) {
            Triangle t = triangles[tIt];
            point3d n_t = point3d::cross(vertices[t[1]].p - vertices[t[0]].p,
                                         vertices[t[2]].p - vertices[t[0]].p);
            normales[ t[0] ].p += n_t;
            normales[ t[1] ].p += n_t;
            normales[ t[2] ].p += n_t;
        }
        for(unsigned int nIt = 0; nIt < normales.size(); ++nIt) {
            if (normales[nIt].p.x() != 0
                || normales[nIt].p.y() != 0
                || normales[nIt].p.z() != 0){
                normales[nIt].p.normalize();
            }
        }
    }

    void computeUV(){}

    void initVAO(QOpenGLShaderProgram* program) {

        program->bind();

        vao = new QOpenGLVertexArrayObject();
        if (!vao->create()){
            program->release();
            qWarning() << "Could not create VAO!";
            return;
        }
        vao->bind();

        vbo_vertex = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        if (!vbo_vertex->create()){
            qWarning() << "Could not create vertex VBO!";
            program->release();
            return;
        }
        vbo_vertex->bind();
        vbo_vertex->setUsagePattern(QOpenGLBuffer::StaticDraw);
        vbo_vertex->allocate(
                    vertices.data(), vertices.size() * sizeof(point3d));
        vbo_vertex->release();

        vbo_normales = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        if (!vbo_normales->create()){
            qWarning() << "Could not create vertex VBO!";
            program->release();
            return;
        }
        vbo_normales->bind();
        vbo_normales->setUsagePattern(QOpenGLBuffer::StaticDraw);
        vbo_normales->allocate(
                    normales.data(), normales.size() * sizeof(point3d));

//        vbo_uv = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
//        if (!vbo_uv->create()){
//            qWarning() << "Could not create vertex VBO!";
//            program->release();
//            return;
//        }
//        vbo_uv->bind();
//        vbo_uv->setUsagePattern(QOpenGLBuffer::StaticDraw);
//        vbo_uv->allocate(
//                    uv.data(), uv.size() * sizeof(QVector2D));

        vbo_triangles = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
        if (!vbo_triangles->create()){
            qWarning() << "Could not create triangles VBO!";
            program->release();
            return;
        }
        vbo_triangles->bind();
        vbo_triangles->setUsagePattern(QOpenGLBuffer::StaticDraw);
        vbo_triangles->allocate(
                    triangles.data(), triangles.size() * 3 * sizeof(uint));

        vbo_vertex->bind();
        program->enableAttributeArray(0);
        program->setAttributeBuffer(
                    0, GL_DOUBLE, offsetof(Vertex, p), 3, sizeof(Vertex));

        vbo_normales->bind();
        program->enableAttributeArray(1);
        program->setAttributeBuffer(
                    1, GL_DOUBLE, offsetof(Vertex, p), 3, sizeof(Vertex));

//        program->enableAttributeArray(2);
//        program->setAttributeBuffer(
//                    2, GL_DOUBLE, sizeof(QVector2D), 3, sizeof(QVector2D));

        vao->release();
        program->release();
    }

    void render(QOpenGLShaderProgram* program) {
        program->bind();
        if (vao == nullptr){
            qWarning() << "VAO is not initialized";
            return;
        }
        vao->bind();

        glDrawElements(
                    GL_TRIANGLES,
                    triangles.size() * 3,
                    GL_UNSIGNED_INT,
                    nullptr);

        vao->release();
        program->release();
    }

    void clear() {
        vao->destroy();
        vbo_vertex->destroy();
        vbo_normales->destroy();
        vbo_triangles->destroy();
    }
};



#endif // PROJECTMESH_H
