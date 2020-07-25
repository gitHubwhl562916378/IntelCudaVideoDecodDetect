extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/pixdesc.h"
#include "libavutil/opt.h"
#include "libavutil/avassert.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}
#include <QDateTime>
#include "ffmpegcudadecode.h"

FFmpegCudaDecode::FFmpegCudaDecode(DecodeTaskManagerImpl *taskManger, QObject *parent):
    DecodeTask(parent),
    taskManager_(taskManger)
{
}

FFmpegCudaDecode::~FFmpegCudaDecode()
{
    stop();

    if(buffer_)
    {
        av_free(buffer_);
        buffer_ = nullptr;
    }
}

void FFmpegCudaDecode::startPlay(QString url)
{
    if(isRunning())
    {
        stop();
    }
    url_ = url;
    start();
}

void FFmpegCudaDecode::stop()
{
    requestInterruption();
    quit();
    wait();
}

int FFmpegCudaDecode::fps()
{
    return fps_;
}

int FFmpegCudaDecode::curFps()
{
    return curFps_;
}

void FFmpegCudaDecode::run()
{
    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext *pCodecCtx = nullptr;
    AVCodec *pCodec = nullptr;
    AVFrame *pFrame = nullptr, *swFrame = nullptr;
    uint8_t *out_buffer;
    AVPacket packet;
    AVStream *video = nullptr;
    char errorbuf[1024]{0};
    QString errorMsg;
    int videoStream, i;
    int ret;

    const char *device_name = "cuda";
    enum AVHWDeviceType type = av_hwdevice_find_type_by_name(device_name);
    if(type == AV_HWDEVICE_TYPE_NONE)
    {
        QString str = "Available device types:";
        while((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
            str += QString(" %1").arg(av_hwdevice_get_type_name(type));
        errorMsg = QString("Device type %1 is not supported.%2").arg(device_name, str);

        emit sigError(errorMsg);
        return;
    }

    AVDictionary *opt = nullptr;
    //av_dict_set(&opt,"buffer_size","1024000",0);
    //av_dict_set(&opt,"max_delay","0",0);
    av_dict_set(&opt,"rtsp_transport","tcp",0);
    av_dict_set(&opt,"stimeout","5000000",0);
    if ((ret = avformat_open_input(&pFormatCtx, url_.toUtf8().data(), nullptr, &opt)) != 0) {
        av_strerror(ret, errorbuf, sizeof(errorbuf));
        errorMsg = errorbuf;
        emit sigError(errorMsg);
        goto  END;
    }

    if ((ret = avformat_find_stream_info(pFormatCtx, nullptr)) < 0) {
        av_strerror(ret, errorbuf, sizeof(errorbuf));
        errorMsg = errorbuf;
        emit sigError(errorMsg);
        goto  END;
    }

    if((ret = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pCodec, 0)) < 0){
        av_strerror(ret, errorbuf, sizeof(errorbuf));
        errorMsg = errorbuf;
        emit sigError(errorMsg);
        goto  END;
    }
    videoStream = ret;

    for (i = 0;; i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(pCodec, i);
        if (!config) {
            errorMsg = QString("Decoder %1 does not support device type %2").arg(pCodec->name).arg(av_hwdevice_get_type_name(type));
            emit sigError(errorMsg);
            goto  END;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                config->device_type == type) {
            hw_pix_fmt = config->pix_fmt;
            break;
        }
    }

    if (!(pCodecCtx = avcodec_alloc_context3(pCodec)))
    {
        errorMsg = QString("avcodec_alloc_context3(pCodec) error: %1").arg(AVERROR(ENOMEM));
        emit sigError(errorMsg);
        goto  END;
    }

    video = pFormatCtx->streams[videoStream];
    if ((ret = avcodec_parameters_to_context(pCodecCtx, video->codecpar)) < 0)
    {
        av_strerror(ret, errorbuf, sizeof(errorbuf));
        errorMsg = errorbuf;
        emit sigError(errorMsg);
        goto  END;
    }

    int vden = video->avg_frame_rate.den,vnum = video->avg_frame_rate.num;
    fps_ = vnum/vden;
    stream_time_base_ = video->time_base;

    pCodecCtx->get_format = get_cuda_hw_format;

    if ((ret = taskManager_->hwDecoderInit(type)) < 0)
    {
        av_strerror(ret, errorbuf, sizeof(errorbuf));
        errorMsg = errorbuf;
        emit sigError(errorMsg);
        goto  END;
    }
    pCodecCtx->hw_device_ctx = av_buffer_ref(taskManager_->hwDecoderBuffer(type));

    ///打开解码器
    if ((ret = avcodec_open2(pCodecCtx, pCodec, nullptr)) < 0) {
        av_strerror(ret, errorbuf, sizeof(errorbuf));
        errorMsg = errorbuf;
        emit sigError(errorMsg);
        goto  END;
    }

    pFrame = av_frame_alloc();
    swFrame = av_frame_alloc();
    av_dump_format(pFormatCtx, 0, url_.toUtf8().data(), 0); //输出视频信息

    while (!isInterruptionRequested() && ret >= 0)
    {
        if ((ret = av_read_frame(pFormatCtx, &packet)) < 0)
        {
            if(ret != AVERROR_EOF)
            {
                av_strerror(ret, errorbuf, sizeof(errorbuf));
                errorMsg = errorbuf;
                emit sigError(errorMsg);
            }
            break; //这里认为视频读取完了
        }

        if (packet.stream_index == videoStream) {
            ret = decode_packet(pCodecCtx, &packet, pFrame, swFrame);
        }

        qint64 end_pt = QDateTime::currentMSecsSinceEpoch();
        if((end_pt - start_pt_) >= 1000)
        {
            if(decode_frames_ != curFps_)
            {
                curFps_ = decode_frames_;
                emit sigCurFpsChanged(curFps_);
            }
            start_pt_ = end_pt;
            decode_frames_ = 0;
        }
        decode_frames_++;

        av_packet_unref(&packet);
    }
    packet.data = nullptr;
    packet.size = 0;
    ret = decode_packet(pCodecCtx, &packet, pFrame, swFrame);

END:
    if(pFrame)
    {
        av_frame_free(&pFrame);
    }
    if(swFrame)
    {
        av_frame_free(&swFrame);
    }
    if(pCodecCtx)
    {
        avcodec_free_context(&pCodecCtx);
    }
    if(pFormatCtx)
    {
        avformat_close_input(&pFormatCtx);
    }
    isDecodeStarted_ = false;
    fps_ = 0;
}

int FFmpegCudaDecode::decode_packet(AVCodecContext *pCodecCtx, AVPacket *packet, AVFrame *pFrame, AVFrame *swFrame)
{
    int ret;
    QString errorMsg;
    char errorbuf[1024]{0};

    if((ret = avcodec_send_packet(pCodecCtx, packet)) < 0 ){
        av_strerror(ret, errorbuf, sizeof(errorbuf));
        errorMsg = QString("Error during avcodec_send_packet: %1").arg(errorbuf);
        emit sigError(errorMsg);
        return ret;
    }

    while (true) {
        ret = avcodec_receive_frame(pCodecCtx, pFrame);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        }else if(ret < 0){
            av_strerror(ret, errorbuf, sizeof(errorbuf));
            errorMsg = QString("Error while avcodec_receive_frame: %1").arg(errorbuf);
            emit sigError(errorMsg);
            goto fail;
        }

        if((ret = av_hwframe_transfer_data(swFrame, pFrame, 0)) < 0 ){
            av_strerror(ret, errorbuf, sizeof(errorbuf));
            errorMsg = QString("Error transferring the data to system memory: %1").arg(errorbuf);
            emit sigError(errorMsg);
            goto fail;
        }
        swFrame->pts =  pFrame->best_effort_timestamp;
        if(swFrame->pts != AV_NOPTS_VALUE)
        {
            if(last_pts_ != AV_NOPTS_VALUE)
            {
                AVRational ra;
                ra.num = 1;
                ra.den = AV_TIME_BASE;
                int64_t delay = av_rescale_q(swFrame->pts - last_pts_, stream_time_base_, ra);
                if(delay > 0 && delay < 1000000)
                {
                    QThread::usleep(delay);
                }
            }
            last_pts_ = swFrame->pts;
        }
        if(!isDecodeStarted_)
        {
            bufferSize_ = av_image_get_buffer_size(AVPixelFormat(swFrame->format), swFrame->width,
                                                  swFrame->height, 1);

            if(buffer_)
            {
                av_free(buffer_);
                buffer_ = nullptr;
            }
            buffer_ = (uint8_t*) av_malloc(bufferSize_);
            emit sigVideoStarted(buffer_, swFrame->format, swFrame->width, swFrame->height);
            isDecodeStarted_ = true;
        }
        //gpu拷贝到cpu，相对耗时
        ret = av_image_copy_to_buffer(buffer_, bufferSize_,
                    (const uint8_t * const *)swFrame->data,
                    (const int *)swFrame->linesize, AVPixelFormat(swFrame->format),
                    swFrame->width, swFrame->height, 1);
        emit sigFrameLoaded();

    fail:
        if(ret < 0){
            return  ret;
        }
    }

    return 0;
}
