/*
 * @Author: whl
 * @Date: 2020-08-02 11:06:45
 * @LastEditTime: 2020-08-03 14:18:35
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \vs_code\Nv12Render_Gpu\nv12render_gpu.h
 */
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <cuda.h>
#include <cudaGL.h>
#include "videorender.h"

class Nv12Render_Gpu : public QOpenGLFunctions, public VideoRender
{
public:
    Nv12Render_Gpu();
    ~Nv12Render_Gpu();
    /**
     * @description: 初始化渲染器
     * @param width 视频宽度
     * @param height 视频宽度
     * @param horizontal 是否水平镜像
     * @param vertical 是否垂直镜像
     */
    void initialize(const int width, const int height, const bool horizontal = false, const bool vertical = false) override;
    /**
     * @description: 渲染一帧数据
     * @param buffer 设备指针
     * @param width 帧宽度
     * @param height 帧高度
     */
    void render(unsigned char* buffer, const int width, const int height) override;
    /**
     * @description: 渲染一帧分离在2个planr的nv12数据
     * @param planr 2个平面地址的指针数组。按照默认格式排序0(Y分量)、1(UV分量)，2个平面
     * @param line_size 二维图片的每行字节大小，2个平面，也是GPU内存的nPitch，顺序和planr保持一致
     * @param width 视频帧宽度
     * @param height 视频帧高度
     */
    void render(unsigned char* planr[], int line_size[], const int width, const int height) override;

private:
    CUgraphicsResource cuda_ybuffer_resource, cuda_uvbuffer_resource;
    CUcontext context = nullptr;
    QOpenGLShaderProgram program;
    GLuint idY,idUV, textures[2];
    GLuint ybuffer_id, uvbuffer_id, tex_buffers[2];
    QOpenGLBuffer vbo;
};
