extern "C"
{
#include "libavutil/hwcontext_qsv.h"
}
#include <QLibrary>
#include "../renderthread.h"
#include "ffmpegcudadecode.h"
#include "ffmpegqsvdecode.h"
#include "ffmpegcpudecode.h"
#include "nvidiadecode.h"

static DecodeTaskManagerImpl* g_task_manager = nullptr;
DecodeTaskManagerImpl::DecodeTaskManagerImpl()
{
    if(!g_task_manager)
    {
        g_task_manager = this;
    }
}

DecodeTaskManagerImpl::~DecodeTaskManagerImpl()
{
}

int DecodeTaskManagerImpl::hwDecoderInit(const AVHWDeviceType fmt)
{
    std::lock_guard<std::mutex> lock(hw_buf_mtx_);
    auto iter = hw_buffer_map_.find(fmt);
    if(iter == hw_buffer_map_.end())
    {
        int ret = 0;
        AVBufferRef *buffer = nullptr;
        switch (fmt) {
        case AV_HWDEVICE_TYPE_CUDA:
            ret = av_hwdevice_ctx_create(&buffer, AV_HWDEVICE_TYPE_CUDA,
                    nullptr, nullptr, 0);
            break;
        case AV_HWDEVICE_TYPE_QSV:
            ret = av_hwdevice_ctx_create(&buffer, AV_HWDEVICE_TYPE_QSV,
                "auto", nullptr, 0);
            break;
        default:
            break;
        }
        if(ret == 0)
        {
            hw_buffer_map_.insert(std::make_pair(fmt, buffer));
        }
        return ret;
    }

    return 0;
}

AVBufferRef *DecodeTaskManagerImpl::hwDecoderBuffer(const AVHWDeviceType type)
{
    std::lock_guard<std::mutex> lock(hw_buf_mtx_);
    auto iter = hw_buffer_map_.find(type);
    if(iter == hw_buffer_map_.end()){
        return nullptr;
    }

    return iter->second;
}

DecodeTask *DecodeTaskManagerImpl::makeTask(RenderThread *render_thr, const QString &device)
{
    if(device == "cuda")
    {
        return new FFmpegCudaDecode(this, render_thr);
    }else if(device == "qsv")
    {
        return new FFmpegQsvDecode(this, render_thr);
    }else if(device == "cpu")
    {
        return new FFmpegCpuDecode(this, render_thr);
    }else if(device == "cuda_plugin")
    {
        std::lock_guard<std::mutex> lock(decoder_plugin_mtx_);
        CreateDecoderFunc func = nullptr;
        auto iter = decoder_plug_map_.find(device);
        if(iter == decoder_plug_map_.end())
        {
            QLibrary dllLoad("NvidiaDecoderPlugin");
            if(dllLoad.load()){
                func = (CreateDecoderFunc)dllLoad.resolve("createDecoder");
                bool code = decoder_plug_map_.insert(std::make_pair(device, func)).second;
                if(!code){
                    render_thr->sigError("load NvidiaDecoderPlugin failed");
                }
            }else{
                render_thr->sigError("load NvidiaDecoderPlugin failed");
            }
        }else{
            func = iter->second;
        }
        return new NvidiaDecode(this, func, render_thr);
    }

    return nullptr;
}

AVPixelFormat get_cuda_hw_format(AVCodecContext *ctx, const AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == AV_PIX_FMT_CUDA)
            return *p;
    }

    fprintf(stderr, "Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}

AVPixelFormat get_qsv_hw_format(AVCodecContext *avctx, const AVPixelFormat *pix_fmts)
{
    while (*pix_fmts != AV_PIX_FMT_NONE) {
        if (*pix_fmts == AV_PIX_FMT_QSV) {
            AVHWFramesContext  *frames_ctx;
            AVQSVFramesContext *frames_hwctx;
            int ret;

            /* create a pool of surfaces to be used by the decoder */
            avctx->hw_frames_ctx = av_hwframe_ctx_alloc(g_task_manager->hwDecoderBuffer(AV_HWDEVICE_TYPE_QSV));
            if (!avctx->hw_frames_ctx)
                return AV_PIX_FMT_NONE;
            frames_ctx = (AVHWFramesContext*)avctx->hw_frames_ctx->data;
            frames_hwctx = (AVQSVFramesContext*) frames_ctx->hwctx;

            frames_ctx->format = AV_PIX_FMT_QSV;
            frames_ctx->sw_format = avctx->sw_pix_fmt;
            frames_ctx->width = FFALIGN(avctx->coded_width, 32);
            frames_ctx->height = FFALIGN(avctx->coded_height, 32);
            frames_ctx->initial_pool_size = 32;

            frames_hwctx->frame_type = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;

            ret = av_hwframe_ctx_init(avctx->hw_frames_ctx);
            if (ret < 0)
                return AV_PIX_FMT_NONE;

            return AV_PIX_FMT_QSV;
        }

        pix_fmts++;
    }

    fprintf(stderr, "The QSV pixel format not offered in get_format()\n");

    return AV_PIX_FMT_NONE;
}
