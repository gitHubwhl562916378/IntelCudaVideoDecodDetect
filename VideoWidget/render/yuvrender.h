#ifndef YUVRENDER_H
#define YUVRENDER_H
#include "videorender.h"
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class YuvRender : public QOpenGLFunctions , public VideoRender
{
public:
    YuvRender() = default;
    YuvRender(const YuvRender &) = delete;
    ~YuvRender() override;
    void initialize(bool horizontal = false, bool vertical = false) override;
    void render(uchar*yuvPtr, int w, int h) override;

private:
    QOpenGLShaderProgram program;
    GLuint idY,idU,idV,textures[3];
    QOpenGLBuffer vbo;
};

#endif // YUVRENDER_H
