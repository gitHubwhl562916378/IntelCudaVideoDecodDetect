#ifndef VIDEORENDER_H
#define VIDEORENDER_H

extern "C" {
#include "libavutil/pixfmt.h"
}

class VideoRender
{
public:
    virtual ~VideoRender(){}
    /**
     * @description: 初始化opengl上下文，编译链接shader;如果是GPU直接与OOPENGL对接数据，则会分配GPU内存或注册资源
     * @param width 视频宽度
     * @param height 视频高度
     * @param horizontal 是否水平镜像
     * @param vertical 是否垂直镜像
     */
    virtual void initialize(const int width, const int height, const bool horizontal = false, const bool vertical = false) = 0;
    /**
     * @description: 渲染一帧数据，buffer需要为连续空间
     * @param buffer 内存地址
     * @param width 视频帧宽度
     * @param height 视频帧高度
     */
    virtual void render(unsigned char* buffer, const int width, const int height) = 0;
    /**
     * @description: 渲染一帧分离在多个planr的数据
     * @param planr 多个平面地址的指针数组。按照默认格式排序，如YUV为0(Y分量)、1(U分量)、2(V分量); NV12为0(Y分量)、1(UV分量)
     * @param line_size 二维图片的每行字节大小，也是GPU内存的nPitch
     * @param width 视频帧宽度
     * @param height 视频帧高度
     */
    virtual void render(unsigned char* planr[], const int line_size[], const int width, const int height) = 0;
    /**
     * @description: 异步加载数据到纹理
     * @param buffer 连续内存地址
     * @param width 视频帧宽度
     * @param height 视频帧高度
     */
    virtual void upLoad(unsigned char* buffer, const int width, const int height) = 0;
    /**
     * @description: 异步加载数一个分散在多个planr的数据到纹理
     * @param planr 多个平面地址的指针数组。按照默认格式排序，如YUV为0(Y分量)、1(U分量)、2(V分量); NV12为0(Y分量)、1(UV分量)
     * @param line_size 二维图片的每行字节大小，也是GPU内存的nPitch
     * @param width 视频帧宽度
     * @param height 视频帧高度
     */
    virtual void upLoad(unsigned char* planr[], const int line_size[], const int width, const int height) = 0;
    /**
     * @description: 异步绘制纹理数据
     */
    virtual void draw() = 0;
};

typedef VideoRender* (*CreateRenderFunc)(void *ctx);
#endif // VIDEORENDER_H
