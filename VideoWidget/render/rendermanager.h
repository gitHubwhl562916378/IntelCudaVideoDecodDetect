#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H
#include "videorender.h"
#include <map>

class RenderManager : public VideoRenderManager
{
public:
    RenderManager() = default;
    RenderManager(const RenderManager&) = delete;
    ~RenderManager() override;
    bool registerRender(AVPixelFormat,bool horizontal = false, bool vertical = false) override;
    bool hasRender(AVPixelFormat) override;
    void render(AVPixelFormat,unsigned char*,int,int) override;

private:
    std::map<AVPixelFormat,VideoRender*> m_rendermap;
};

#endif // RENDERMANAGER_H
