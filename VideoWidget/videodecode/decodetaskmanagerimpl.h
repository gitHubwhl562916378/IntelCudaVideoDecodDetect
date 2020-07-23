#ifndef DECODETASKMANAGERIMPL_H
#define DECODETASKMANAGERIMPL_H

#include <functional>
#include <mutex>
extern "C"
{
#include "libavcodec/avcodec.h"
}
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
    DecodeTask* makeTask(const QString) override;

private:
    std::mutex hw_buf_mtx_;
    std::map<AVHWDeviceType, AVBufferRef*> hw_buffer_map_;
};

#endif // DECODETASKMANAGERIMPL_H
