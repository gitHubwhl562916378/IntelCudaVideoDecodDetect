#include "rendermanager.h"
#include "yuvrender.h"
#include "nv12render.h"
#include <iostream>

RenderManager::~RenderManager()
{
    auto iter = m_rendermap.begin();
    for(;iter != m_rendermap.end();){
        delete iter->second;
        iter = m_rendermap.erase(iter);
    }
}

bool RenderManager::registerRender(AVPixelFormat type, bool horizontal, bool vertical)
{
    VideoRender *r{nullptr};
    switch (type) {
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUVJ420P:
        r = new YuvRender;
        break;
    case AV_PIX_FMT_NV12:
        r = new Nv12Render;
        break;
    default:
        break;
    }

    if(!r){
        std::cout << "no render for this format" << std::endl;
        return false;
    }

    r->initialize(horizontal,vertical);
    std::cout << "render registered " << type << std::endl;
    m_rendermap.insert(std::make_pair(type,r));
    return true;
}

bool RenderManager::hasRender(AVPixelFormat format)
{
    return m_rendermap.find(format) != m_rendermap.end();
}

void RenderManager::render(AVPixelFormat type, unsigned char *ptr, int w, int h)
{
    auto iter = m_rendermap.begin();
    for(;iter != m_rendermap.end(); iter++){
        if(iter->first == type){
            break;
        }
    }

    if(iter != m_rendermap.end()){
        iter->second->render(ptr,w,h);
    }
}
