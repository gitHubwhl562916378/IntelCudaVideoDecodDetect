#ifndef DECODETASKMANAGERIMPL_H
#define DECODETASKMANAGERIMPL_H

#include <functional>
#include <map>
#include <mutex>
extern "C"
{
#include "libavcodec/avcodec.h"
}
#include "nvidia_decoer_api.h"
#include "decodtask.h"

AVPixelFormat get_cuda_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);
AVPixelFormat get_qsv_hw_format(AVCodecContext *avctx, const enum AVPixelFormat *pix_fmts);

class DecodeTaskManagerImpl : public DecodeTaskManager
{
public:
    DecodeTaskManagerImpl();
    ~DecodeTaskManagerImpl() override;
    int hwDecoderInit(const enum AVHWDeviceType);
    AVBufferRef *hwDecoderBuffer(const AVHWDeviceType);
    DecodeTask* makeTask(RenderThread*render_thr, const QString &device) override;

private:
    std::mutex hw_buf_mtx_, decoder_plugin_mtx_;
    std::map<AVHWDeviceType, AVBufferRef*> hw_buffer_map_;
    std::map<QString, CreateDecoderFunc> decoder_plug_map_;
};

#endif // DECODETASKMANAGERIMPL_H
