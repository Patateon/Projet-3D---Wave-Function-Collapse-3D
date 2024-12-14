TEMPLATE = app
TARGET = wfc
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ./src
QT += opengl xml
CONFIG += qt \
    release
MOC_DIR = ./tmp/moc
OBJECTS_DIR = ./tmp/obj

# Input
HEADERS += ./src/point3.h \
    ./src/Mesh.h \
    ./src/BasicIO.h \
    ./src/gl/GLUtilityMethods.h \
    ./src/gl/BasicColors.h \
    ./src/tilemodel.h \
    extern/objloader/OBJ_Loader.h \
    src/mainviewer.h \
    src/material.h \
    src/tileinstance.h \
    src/transform.h \
    ./src/grid.h \
    src/wfc.h
SOURCES += ./src/main.cpp \
    ./src/gl/GLUtilityMethods.cpp\
    ./src/gl/BasicColors.cpp \
    ./src/tilemodel.cpp \
    src/mainviewer.cpp \
    src/material.cpp \
    src/tileinstance.cpp \
    ./src/grid.cpp \
    src/transform.cpp \
    src/wfc.cpp

RESOURCES += shaders/


EXT_DIR = extern



#QGLViewer
{
 INCLUDEPATH += $${EXT_DIR}/libQGLViewer-2.6.1
 LIBS +=    -L$${EXT_DIR}/libQGLViewer-2.6.1/QGLViewer \
            -lQGLViewer-qt5
}



LIBS += -lglut \
    -lGLU
LIBS += -lgsl \
    -lgomp
LIBS += -lblas \
    -lgomp
release:QMAKE_CXXFLAGS_RELEASE += -O3 \
    -fopenmp
release:QMAKE_CFLAGS_RELEASE += -O3 \
    -fopenmp

DISTFILES += \
    shaders/gshader.glsl \
    src/shader/fshader.glsl \
    src/shader/vshader.glsl
