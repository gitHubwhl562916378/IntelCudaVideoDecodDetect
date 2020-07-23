#include "yuvrender.h"
#include <iostream>
YuvRender::~YuvRender()
{
    vbo.destroy();
    glDeleteTextures(sizeof(textures) / sizeof(GLuint),textures);
}

void YuvRender::initialize(bool horizontal, bool vertical)
{
    initializeOpenGLFunctions();
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
}

void YuvRender::render(uchar *yuvPtr, int w, int h)
{
    glDisable(GL_DEPTH_TEST);
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
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,w,h,0,GL_RED,GL_UNSIGNED_BYTE,yuvPtr);
    //https://blog.csdn.net/xipiaoyouzi/article/details/53584798 纹理参数解析
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D,idU);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,w >> 1, h >> 1,0,GL_RED,GL_UNSIGNED_BYTE,yuvPtr + w * h);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D,idV);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w >> 1, h >> 1, 0, GL_RED, GL_UNSIGNED_BYTE, yuvPtr+w*h*5/4);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    program.setUniformValue("tex_y",2);
    program.setUniformValue("tex_u",1);
    program.setUniformValue("tex_v",0);
    glDrawArrays(GL_QUADS,0,4);
    program.disableAttributeArray("vertexIn");
    program.disableAttributeArray("textureIn");
    vbo.release();
    program.release();
}
