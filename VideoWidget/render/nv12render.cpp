#include "nv12render.h"
#include <QOpenGLTexture>
#include <QMutexLocker>
#include <QDebug>

Nv12Render::~Nv12Render()
{
    vbo.destroy();
    glDeleteTextures(sizeof(textures) / sizeof(GLuint),textures);
    if(buffer_){
        delete buffer_;
        buffer_ = nullptr;
    }
}

Q_GLOBAL_STATIC(QMutex, initMutex)
void Nv12Render::initialize(const int width, const int height, const bool horizontal, const bool vertical)
{
    initializeOpenGLFunctions();
    QMutexLocker initLock(initMutex());
    const char *vsrc =
            "attribute vec4 vertexIn; \
             attribute vec4 textureIn; \
             varying vec4 textureOut;  \
             void main(void)           \
             {                         \
                 gl_Position = vertexIn; \
                 textureOut = textureIn; \
             }";

    const char *fsrc =
            "varying mediump vec4 textureOut;\n"
            "uniform sampler2D textureY;\n"
            "uniform sampler2D textureUV;\n"
            "void main(void)\n"
            "{\n"
            "vec3 yuv; \n"
            "vec3 rgb; \n"
            "yuv.x = texture2D(textureY, textureOut.st).r - 0.0625; \n"
            "yuv.y = texture2D(textureUV, textureOut.st).r - 0.5; \n"
            "yuv.z = texture2D(textureUV, textureOut.st).g - 0.5; \n"
            "rgb = mat3( 1,       1,         1, \n"
                        "0,       -0.39465,  2.03211, \n"
                        "1.13983, -0.58060,  0) * yuv; \n"
            "gl_FragColor = vec4(rgb, 1); \n"
            "}\n";

    program.addCacheableShaderFromSourceCode(QOpenGLShader::Vertex,vsrc);
    program.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment,fsrc);
    program.link();

    if(horizontal){
        if(vertical){
            GLfloat points[]{
                -1.0f, 1.0f,
                 1.0f, 1.0f,
                 1.0f, -1.0f,
                -1.0f, -1.0f,

                1.0f,1.0f,
                0.0f,1.0f,
                0.0f,0.0f,
                1.0f,0.0f
            };

            vbo.create();
            vbo.bind();
            vbo.allocate(points,sizeof(points));
        }else{
            GLfloat points[]{
                -1.0f, 1.0f,
                 1.0f, 1.0f,
                 1.0f, -1.0f,
                -1.0f, -1.0f,

                1.0f,0.0f,
                0.0f,0.0f,
                0.0f,1.0f,
                1.0f,1.0f
            };

            vbo.create();
            vbo.bind();
            vbo.allocate(points,sizeof(points));
        }
    }else{
        if(vertical){
            GLfloat points[]{
                -1.0f, 1.0f,
                 1.0f, 1.0f,
                 1.0f, -1.0f,
                -1.0f, -1.0f,

                0.0f,1.0f,
                1.0f,1.0f,
                1.0f,0.0f,
                0.0f,0.0f
            };

            vbo.create();
            vbo.bind();
            vbo.allocate(points,sizeof(points));
        }else{
            GLfloat points[]{
                -1.0f, 1.0f,
                 1.0f, 1.0f,
                 1.0f, -1.0f,
                -1.0f, -1.0f,

                0.0f,0.0f,
                1.0f,0.0f,
                1.0f,1.0f,
                0.0f,1.0f
            };

            vbo.create();
            vbo.bind();
            vbo.allocate(points,sizeof(points));
        }
    }

    GLuint id[2];
    glGenTextures(2,id);
    idY = id[0];
    idUV = id[1];
    std::copy(std::begin(id),std::end(id),std::begin(textures));

    glBindTexture(GL_TEXTURE_2D,idY);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width,height,0,GL_RED,GL_UNSIGNED_BYTE,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glBindTexture(GL_TEXTURE_2D,idUV);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RG,width >> 1,height >> 1,0,GL_RG,GL_UNSIGNED_BYTE,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Nv12Render::render(uchar *nv12Ptr, const int w, const int h)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(false);
    if(!nv12Ptr){
        return;
    }

    program.bind();
    vbo.bind();
    program.enableAttributeArray("vertexIn");
    program.enableAttributeArray("textureIn");
    program.setAttributeBuffer("vertexIn",GL_FLOAT, 0, 2, 2*sizeof(GLfloat));
    program.setAttributeBuffer("textureIn",GL_FLOAT,2 * 4 * sizeof(GLfloat),2,2*sizeof(GLfloat));

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D,idY);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, nv12Ptr);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D,idUV);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w >> 1,h >> 1, GL_RG, GL_UNSIGNED_BYTE, nv12Ptr + w*h);

    program.setUniformValue("textureY",1);
    program.setUniformValue("textureUV",0);
    glDrawArrays(GL_QUADS,0,4);
    program.disableAttributeArray("vertexIn");
    program.disableAttributeArray("textureIn");
    vbo.release();
    program.release();
}

void Nv12Render::render(unsigned char *planr[], const int line_size[], const int width, const int height)
{
    if(!planr){
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDepthMask(false);
        return;
    }

    if(!buffer_){
        buffer_ = new unsigned char[width * height * 3 / 2];
    }

    int bytes = 0; //yuv data有3块内存分别拷，nv12只有2块内存分别拷
    for(int i = 0; i <height; i++){ //将y分量拷贝
        ::memcpy(buffer_ + bytes,planr[0] + line_size[0] * i, width);
        bytes += width;
    }
    int uv = height >> 1;
    for(int i = 0; i < uv; i++){ //将u分量拷贝
        ::memcpy(buffer_ + bytes,planr[1] + line_size[1] * i, width);
        bytes += width;
    }
    render(buffer_, width, height);
}

void Nv12Render::upLoad(unsigned char *buffer, const int w, const int h)
{
    if(!buffer){
        return;
    }

    QMutexLocker lock(&mtx);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D,idY);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, buffer);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D,idUV);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w >> 1,h >> 1, GL_RG, GL_UNSIGNED_BYTE, buffer + w*h);
    glFlush();
}

void Nv12Render::upLoad(unsigned char* planr[], const int line_size[], const int width, const int height)
{
    if(!planr){
        return;
    }

    if(!buffer_){
        buffer_ = new unsigned char[width * height * 3 / 2];
    }

    int bytes = 0; //yuv data有3块内存分别拷，nv12只有2块内存分别拷
    for(int i = 0; i <height; i++){ //将y分量拷贝
        ::memcpy(buffer_ + bytes,planr[0] + line_size[0] * i, width);
        bytes += width;
    }
    int uv = height >> 1;
    for(int i = 0; i < uv; i++){ //将u分量拷贝
        ::memcpy(buffer_ + bytes,planr[1] + line_size[1] * i, width);
        bytes += width;
    }
    upLoad(buffer_, width, height);
}

void Nv12Render::draw()
{
    QMutexLocker lock(&mtx);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(false);

    program.bind();
    vbo.bind();
    program.enableAttributeArray("vertexIn");
    program.enableAttributeArray("textureIn");
    program.setAttributeBuffer("vertexIn",GL_FLOAT, 0, 2, 2*sizeof(GLfloat));
    program.setAttributeBuffer("textureIn",GL_FLOAT,2 * 4 * sizeof(GLfloat),2,2*sizeof(GLfloat));

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D,idY);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D,idUV);

    program.setUniformValue("textureY",1);
    program.setUniformValue("textureUV",0);
    glDrawArrays(GL_QUADS,0,4);
    program.disableAttributeArray("vertexIn");
    program.disableAttributeArray("textureIn");
    vbo.release();
    program.release();
}
