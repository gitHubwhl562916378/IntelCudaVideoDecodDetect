#ifndef FFMPEGQSVDECODE_H
#define FFMPEGQSVDECODE_H

#include "decodetaskmanagerimpl.h"
class VideoRender;
class FFmpegQsvDecode : public DecodeTask
{
public:
    FFmpegQsvDecode(DecodeTaskManagerImpl *taskManger, RenderThread *render_thr);
    ~FFmpegQsvDecode();
    void decode(const QString &url) override;

private:
    int decode_packet(AVCodecContext *decoder_ctx,
                      AVFrame *frame, AVFrame *sw_frame,
                      AVPacket *pkt);

    DecodeTaskManagerImpl *taskManager_ = nullptr;
    AVPixelFormat hw_pix_fmt;
    uint8_t *buffer_ = nullptr;
    int bufferSize_ = 0, curFps_ = 0;
    qint64 start_pt_ = 0, last_pts_ = AV_NOPTS_VALUE;
    int decode_frames_ = 0;
    AVRational stream_time_base_;

    VideoRender *render_{nullptr};
};

#endif // FFMPEGQSVDECODE_H
