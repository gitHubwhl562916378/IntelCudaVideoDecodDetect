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
    void initialize(const int width, const int height, const bool horizontal = false, const bool vertical = false) override;
    void render(uchar*yuvPtr, const int w, const int h) override;
    void render(unsigned char* planr[], int line_size[], const int width, const int height) override;

private:
    QOpenGLShaderProgram program;
    GLuint idY,idU,idV,textures[3];
    QOpenGLBuffer vbo;
    unsigned char* buffer_ = nullptr;
};

#endif // YUVRENDER_H
