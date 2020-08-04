#ifndef NVIDIADECODE_H
#define NVIDIADECODE_H

#include "decodetaskmanagerimpl.h"
class VideoRender;
class NvidiaDecode : public DecodeTask
{
public:
    NvidiaDecode(DecodeTaskManagerImpl *taskManger, CreateDecoderFunc func, RenderThread *render_thr);
    ~NvidiaDecode() override;

    void decode(const QString &url) override;

private:
    DecodeTaskManagerImpl *taskManager_ = nullptr;
    CreateDecoderFunc createDecoderFunc_;
    DecoderApi *decoder_ = nullptr;
    VideoRender *render_{nullptr};

    qint64 decode_frames_ = 0, curFps_ = 0, start_pt_ = 0;
};

#endif // NVIDIADECODE_H
