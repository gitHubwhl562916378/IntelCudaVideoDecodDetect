#ifndef NV12RENDER_H
#define NV12RENDER_H
#include "videorender.h"
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class Nv12Render : public QOpenGLFunctions, public VideoRender
{
public:
    Nv12Render() = default;
    Nv12Render(const Nv12Render&) = delete;
    ~Nv12Render() override;
    void initialize(const int width, const int height, const bool horizontal = false, const bool vertical = false) override;
    void render(uchar*nv12Ptr, const int w, const int h) override;
    void render(unsigned char* planr[], int line_size[], const int width, const int height) override;

private:
    QOpenGLShaderProgram program;
    GLuint idY,idUV, textures[2];
    QOpenGLBuffer vbo;
    unsigned char* buffer_ = nullptr;
};

#endif // NV12RENDER_H
