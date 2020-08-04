#ifndef FFMPEGCPUDECODE_H
#define FFMPEGCPUDECODE_H

#include "decodetaskmanagerimpl.h"
class VideoRender;
class FFmpegCpuDecode : public DecodeTask
{
public:
    FFmpegCpuDecode(DecodeTaskManagerImpl *taskManger, RenderThread *render_thr);
    ~FFmpegCpuDecode() override;

    void decode(const QString &url) override;

private:
    int decode_packet(AVCodecContext *avctx, AVPacket *packet, AVFrame *frame);

    DecodeTaskManagerImpl *taskManager_ = nullptr;
    uint8_t *buffer_ = nullptr;
    int bufferSize_ = 0, curFps_ = 0;
    qint64 start_pt_ = 0, last_pts_ = AV_NOPTS_VALUE;
    int decode_frames_ = 0;
    AVRational stream_time_base_;

    VideoRender *render_{nullptr};
};

#endif // FFMPEGCPUDECODE_H
