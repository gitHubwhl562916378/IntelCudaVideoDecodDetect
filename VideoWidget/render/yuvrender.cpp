#include "yuvrender.h"
#include <iostream>
#include <QMutexLocker>
#include <QThread>
YuvRender::~YuvRender()
{
    vbo.destroy();
    glDeleteTextures(sizeof(textures) / sizeof(GLuint),textures);
    if(buffer_){
        delete buffer_;
        buffer_ = nullptr;
    }
}

Q_GLOBAL_STATIC(QMutex, initMutex)
void YuvRender::initialize(const int width, const int height, const bool horizontal, const bool vertical)
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
            "varying mediump vec4 textureOut;\
            uniform sampler2D tex_y; \
            uniform sampler2D tex_u; \
            uniform sampler2D tex_v; \
            void main(void) \
            { \
                vec3 yuv; \
                vec3 rgb; \
                yuv.x = texture2D(tex_y, textureOut.st).r; \
                yuv.y = texture2D(tex_u, textureOut.st).r - 0.5; \
                yuv.z = texture2D(tex_v, textureOut.st).r - 0.5; \
                rgb = mat3( 1,       1,         1, \
                            0,       -0.39465,  2.03211, \
                            1.13983, -0.58060,  0) * yuv; \
                gl_FragColor = vec4(rgb, 1); \
            }";

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

    GLuint id[3];
    glGenTextures(3,id);
    idY = id[0];
    idU = id[1];
    idV = id[2];
    std::copy(std::begin(id),std::end(id),std::begin(textures));

    glBindTexture(GL_TEXTURE_2D,idY);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width,height,0,GL_RED,GL_UNSIGNED_BYTE,nullptr);
    //https://blog.csdn.net/xipiaoyouzi/article/details/53584798 纹理参数解析
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D,idU);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,width >> 1, height >> 1,0,GL_RED,GL_UNSIGNED_BYTE,nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D,idV);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width >> 1, height >> 1, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void YuvRender::render(uchar *yuvPtr, const int w, const int h)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(false);
    if(!yuvPtr){
        return;
    }

    program.bind();
    vbo.bind();
    program.enableAttributeArray("vertexIn");
    program.enableAttributeArray("textureIn");
    program.setAttributeBuffer("vertexIn",GL_FLOAT, 0, 2, 2*sizeof(GLfloat));
    program.setAttributeBuffer("textureIn",GL_FLOAT,2 * 4 * sizeof(GLfloat),2,2*sizeof(GLfloat));

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D,idY);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,w,h,GL_RED,GL_UNSIGNED_BYTE,yuvPtr);
    //https://blog.csdn.net/xipiaoyouzi/article/details/53584798 纹理参数解析

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D,idU);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,w >> 1, h >> 1,GL_RED,GL_UNSIGNED_BYTE,yuvPtr + w * h);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D,idV);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,w >> 1, h >> 1,GL_RED,GL_UNSIGNED_BYTE,yuvPtr + w*h*5/4);

    program.setUniformValue("tex_y",2);
    program.setUniformValue("tex_u",1);
    program.setUniformValue("tex_v",0);
    glDrawArrays(GL_QUADS,0,4);
    program.disableAttributeArray("vertexIn");
    program.disableAttributeArray("textureIn");
    vbo.release();
    program.release();
}

void YuvRender::render(unsigned char *planr[], const int line_size[], const int width, const int height)
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
        ::memcpy(buffer_ + bytes,planr[1] + line_size[1] * i, width >> 1);
        bytes += width >> 1;
    }
    for(int i = 0; i < uv; i++){ //将v分量拷贝
        ::memcpy(buffer_ + bytes,planr[2] + line_size[2] * i, width >> 1);
        bytes += width >> 1;
    }
    render(buffer_, width, height);
}

void YuvRender::upLoad(unsigned char *buffer, const int w, const int h)
{
    if(!buffer){
        return;
    }

    QMutexLocker lock(&mtx);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D,idY);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,w,h,GL_RED,GL_UNSIGNED_BYTE,buffer);
    //https://blog.csdn.net/xipiaoyouzi/article/details/53584798 纹理参数解析

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D,idU);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,w >> 1, h >> 1,GL_RED,GL_UNSIGNED_BYTE,buffer + w * h);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D,idV);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,w >> 1, h >> 1,GL_RED,GL_UNSIGNED_BYTE,buffer + w*h*5/4);
    glFlush();
}

void YuvRender::upLoad(unsigned char *planr[], const int line_size[], const int width, const int height)
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
        ::memcpy(buffer_ + bytes,planr[1] + line_size[1] * i, width >> 1);
        bytes += width >> 1;
    }
    for(int i = 0; i < uv; i++){ //将v分量拷贝
        ::memcpy(buffer_ + bytes,planr[2] + line_size[2] * i, width >> 1);
        bytes += width >> 1;
    }
    upLoad(buffer_, width, height);
}

void YuvRender::draw()
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

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D,idY);

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D,idU);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D,idV);

    program.setUniformValue("tex_y",2);
    program.setUniformValue("tex_u",1);
    program.setUniformValue("tex_v",0);
    glDrawArrays(GL_QUADS,0,4);
    program.disableAttributeArray("vertexIn");
    program.disableAttributeArray("textureIn");
    vbo.release();
    program.release();
}
