/*
 *
 * @Author: your name
 * @Date: 2020-08-02 11:10:34
 * @LastEditTime: 2020-08-03 14:15:47
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \vs_code\Nv12Render_Gpu\nv12render_gpu.cpp
 */

#ifdef _WIN32
#include <Windows.h>
#endif // WIN32
#include "nv12render_gpu.h"

inline bool check(int e, int iLine, const char *szFile) {
    if (e != 0) {
        qDebug() << "General error " << e << " at line " << iLine << " in file " << szFile;
        return false;
    }
    return true;
}

#define ck(call) check(call, __LINE__, __FILE__)

Nv12Render_Gpu::Nv12Render_Gpu()
{
    ck(cuInit(0));
    CUdevice cuDevice;
    ck(cuDeviceGet(&cuDevice, 0));
    char szDeviceName[80];
    ck(cuDeviceGetName(szDeviceName, sizeof(szDeviceName), cuDevice));
    qDebug() << "GPU in use: " << szDeviceName;
    ck(cuCtxCreate(&context, CU_CTX_SCHED_BLOCKING_SYNC, cuDevice));
}

Nv12Render_Gpu::~Nv12Render_Gpu()
{
    ck(cuGraphicsUnregisterResource(cuda_ybuffer_resource));
    ck(cuGraphicsUnregisterResource(cuda_uvbuffer_resource));
    ck(cuCtxDestroy(context));
    vbo.destroy();
    glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);
    glDeleteBuffers(sizeof(tex_buffers)/sizeof(GLuint), tex_buffers);
}

void Nv12Render_Gpu::initialize(const int width, const int height, const bool horizontal, const bool vertical)
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


    glGenBuffers(2, tex_buffers);
    ybuffer_id = tex_buffers[0];
    uvbuffer_id = tex_buffers[1];

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, ybuffer_id);
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, width * height * sizeof(char), nullptr, GL_STREAM_DRAW_ARB);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, uvbuffer_id);
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, width * height* sizeof(char) / 2, nullptr, GL_STREAM_DRAW_ARB);

    ck(cuCtxSetCurrent(context));
    ck(cuGraphicsGLRegisterBuffer(&cuda_ybuffer_resource, ybuffer_id, CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD));
    ck(cuGraphicsGLRegisterBuffer(&cuda_uvbuffer_resource, uvbuffer_id, CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD));
}

void Nv12Render_Gpu::render(unsigned char* nv12_dPtr, const int width, const int height)
{
    if(!nv12_dPtr)
    {
        return;
    }

    ck(cuCtxSetCurrent(context));
    CUdeviceptr d_ybuffer;
    size_t d_y_size;
    ck(cuGraphicsMapResources(1, &cuda_ybuffer_resource, 0));
    ck(cuGraphicsResourceGetMappedPointer(&d_ybuffer, &d_y_size, cuda_ybuffer_resource));
    CUDA_MEMCPY2D m = { 0 };
    m.srcMemoryType = CU_MEMORYTYPE_DEVICE;
    m.srcDevice = reinterpret_cast<CUdeviceptr>(nv12_dPtr);
    m.srcPitch = width;
    m.dstMemoryType = CU_MEMORYTYPE_DEVICE;
    m.dstDevice = d_ybuffer;
    m.dstPitch = d_y_size / height;
    m.WidthInBytes = width;
    m.Height = height;
    ck(cuMemcpy2DAsync(&m, 0));
    ck(cuGraphicsUnmapResources(1, &cuda_ybuffer_resource, 0));

    CUdeviceptr d_uvbuffer;
    size_t d_uv_size;
    ck(cuGraphicsMapResources(1, &cuda_uvbuffer_resource, 0));
    ck(cuGraphicsResourceGetMappedPointer(&d_uvbuffer, &d_uv_size, cuda_uvbuffer_resource));
    m.srcMemoryType = CU_MEMORYTYPE_DEVICE;
    m.srcDevice = reinterpret_cast<CUdeviceptr>(nv12_dPtr + width * height);
    m.srcPitch = width;
    m.dstMemoryType = CU_MEMORYTYPE_DEVICE;
    m.dstDevice = d_uvbuffer;
    m.dstPitch = d_uv_size / (height>>1);
    m.WidthInBytes = width;
    m.Height = (height>>1);
    ck(cuMemcpy2DAsync(&m, 0));
    ck(cuGraphicsUnmapResources(1, &cuda_uvbuffer_resource, 0));

    program.bind();
    vbo.bind();
    program.enableAttributeArray("vertexIn");
    program.enableAttributeArray("textureIn");
    program.setAttributeBuffer("vertexIn",GL_FLOAT, 0, 2, 2*sizeof(GLfloat));
    program.setAttributeBuffer("textureIn",GL_FLOAT,2 * 4 * sizeof(GLfloat),2,2*sizeof(GLfloat));

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, ybuffer_id);
    glBindTexture(GL_TEXTURE_2D,idY);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, uvbuffer_id);
    glBindTexture(GL_TEXTURE_2D,idUV);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width >> 1,height >> 1, GL_RG, GL_UNSIGNED_BYTE, nullptr);

    program.setUniformValue("textureY",1);
    program.setUniformValue("textureUV",0);
    glDrawArrays(GL_QUADS,0,4);
    program.disableAttributeArray("vertexIn");
    program.disableAttributeArray("textureIn");
    vbo.release();
    program.release();
}

void Nv12Render_Gpu::render(unsigned char* planr[], int line_size[], const int width, const int height)
{
    if(!planr)
    {
        return;
    }

    ck(cuCtxSetCurrent(context));
    CUdeviceptr d_ybuffer;
    size_t d_y_size;
    ck(cuGraphicsMapResources(1, &cuda_ybuffer_resource, 0));
    ck(cuGraphicsResourceGetMappedPointer(&d_ybuffer, &d_y_size, cuda_ybuffer_resource));
    CUDA_MEMCPY2D m = { 0 };
    m.srcMemoryType = CU_MEMORYTYPE_DEVICE;
    m.srcDevice = reinterpret_cast<CUdeviceptr>(planr[0]);
    m.srcPitch = line_size[0];
    m.dstMemoryType = CU_MEMORYTYPE_DEVICE;
    m.dstDevice = d_ybuffer;
    m.dstPitch = d_y_size / height;
    m.WidthInBytes = width;
    m.Height = height;
    ck(cuMemcpy2DAsync(&m, 0));
    ck(cuGraphicsUnmapResources(1, &cuda_ybuffer_resource, 0));

    CUdeviceptr d_uvbuffer;
    size_t d_uv_size;
    ck(cuGraphicsMapResources(1, &cuda_uvbuffer_resource, 0));
    ck(cuGraphicsResourceGetMappedPointer(&d_uvbuffer, &d_uv_size, cuda_uvbuffer_resource));
    m.srcMemoryType = CU_MEMORYTYPE_DEVICE;
    m.srcDevice = reinterpret_cast<CUdeviceptr>(planr[1]);
    m.srcPitch = line_size[1];
    m.dstMemoryType = CU_MEMORYTYPE_DEVICE;
    m.dstDevice = d_uvbuffer;
    m.dstPitch = d_uv_size / (height>>1);
    m.WidthInBytes = width;
    m.Height = (height>>1);
    ck(cuMemcpy2DAsync(&m, 0));
    ck(cuGraphicsUnmapResources(1, &cuda_uvbuffer_resource, 0));

    program.bind();
    vbo.bind();
    program.enableAttributeArray("vertexIn");
    program.enableAttributeArray("textureIn");
    program.setAttributeBuffer("vertexIn",GL_FLOAT, 0, 2, 2*sizeof(GLfloat));
    program.setAttributeBuffer("textureIn",GL_FLOAT,2 * 4 * sizeof(GLfloat),2,2*sizeof(GLfloat));

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, ybuffer_id);
    glBindTexture(GL_TEXTURE_2D,idY);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, uvbuffer_id);
    glBindTexture(GL_TEXTURE_2D,idUV);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width >> 1,height >> 1, GL_RG, GL_UNSIGNED_BYTE, nullptr);

    program.setUniformValue("textureY",1);
    program.setUniformValue("textureUV",0);
    glDrawArrays(GL_QUADS,0,4);
    program.disableAttributeArray("vertexIn");
    program.disableAttributeArray("textureIn");
    vbo.release();
    program.release();
}
