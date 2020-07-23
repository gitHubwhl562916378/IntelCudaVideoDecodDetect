#ifndef VIDEORENDER_H
#define VIDEORENDER_H

extern "C" {
#include "libavutil/pixfmt.h"
}

class VideoRender
{
public:
    virtual ~VideoRender(){}
    virtual void initialize(bool horizontal = false, bool vertical = false) = 0;
    virtual void render(unsigned char*,int,int) = 0;
};

class VideoRenderManager
{
public:
   virtual ~VideoRenderManager(){}
   virtual bool registerRender(AVPixelFormat,bool horizontal = false, bool vertical = false) = 0;
   virtual bool hasRender(AVPixelFormat) = 0;
   virtual void render(AVPixelFormat,unsigned char*,int,int) = 0;
};
#endif // VIDEORENDER_H
