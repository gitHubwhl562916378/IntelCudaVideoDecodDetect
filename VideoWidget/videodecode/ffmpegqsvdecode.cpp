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
#include <QDebug>
#include "../renderthread.h"
#include "ffmpegqsvdecode.h"

FFmpegQsvDecode::FFmpegQsvDecode(DecodeTaskManagerImpl *taskManger, RenderThread *render_thr):
    DecodeTask(render_thr),
    taskManager_(taskManger)
{

}

FFmpegQsvDecode::~FFmpegQsvDecode()
{
    if(buffer_)
    {
        av_free(buffer_);
        buffer_ = nullptr;
    }

    qDebug() << "FFmpegQsvDecode::~FFmpegQsvDecode()";
}

void FFmpegQsvDecode::decode(const QString &url)
{
    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext *pCodecCtx = nullptr;
    AVCodec *pCodec = nullptr;
    AVFrame *pFrame = nullptr, *swFrame = nullptr;
    uint8_t *out_buffer;
    AVPacket packet;
    AVStream *video_st = nullptr;
    char errorbuf[1024]{0};
    QString errorMsg;
    int ret, i;

    AVDictionary *opt = nullptr;
    //av_dict_set(&opt,"buffer_size","1024000",0);
    //av_dict_set(&opt,"max_delay","0",0);
    av_dict_set(&opt,"rtsp_transport","tcp",0);
    av_dict_set(&opt,"stimeout","5000000",0);
    if ((ret = avformat_open_input(&pFormatCtx, url.toUtf8().data(), nullptr, &opt)) != 0) {
        av_strerror(ret, errorbuf, sizeof(errorbuf));
        errorMsg = errorbuf;
        thread()->sigError(errorMsg);
        goto  END;
    }

    if ((ret = avformat_find_stream_info(pFormatCtx, nullptr)) < 0) {
        av_strerror(ret, errorbuf, sizeof(errorbuf));
        errorMsg = errorbuf;
        thread()->sigError(errorMsg);
        goto  END;
    }

    /* find the first H.264 video stream */
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        AVStream *st = pFormatCtx->streams[i];

        if (st->codecpar->codec_id == AV_CODEC_ID_H264 && !video_st)//AV_CODEC_ID_HEVC
            video_st = st;
        else
            st->discard = AVDISCARD_ALL;
    }
    if (!video_st) {
        errorMsg = "No H.264 video stream in the input file";
        thread()->sigError(errorMsg);
        goto  END;
    }

    int vden = video_st->avg_frame_rate.den,vnum = video_st->avg_frame_rate.num;
    if(vden > 0)
    {
        thread()->sigFps(vnum/vden);
    }

    if ((ret = taskManager_->hwDecoderInit(AV_HWDEVICE_TYPE_QSV)) < 0)
    {
        av_strerror(ret, errorbuf, sizeof(errorbuf));
        errorMsg = errorbuf;
        thread()->sigError(errorMsg);
        goto  END;
    }

    pCodec = avcodec_find_decoder_by_name("h264_qsv");
    if(!pCodec){
        errorMsg = "The QSV decoder is not present in libavcodec";
        thread()->sigError(errorMsg);
        goto END;
    }

    if (!(pCodecCtx = avcodec_alloc_context3(pCodec)))
    {
        errorMsg = QString("avcodec_alloc_context3(pCodec) error: %1").arg(AVERROR(ENOMEM));
        thread()->sigError(errorMsg);
        goto  END;
    }

    pCodecCtx->codec_id = AV_CODEC_ID_H264;
    if (video_st->codecpar->extradata_size) {
        pCodecCtx->extradata = (uint8_t*)av_mallocz(video_st->codecpar->extradata_size +
            AV_INPUT_BUFFER_PADDING_SIZE);
        if (!pCodecCtx->extradata) {
            ret = AVERROR(ENOMEM);
            goto END;
        }
        memcpy(pCodecCtx->extradata, video_st->codecpar->extradata,
            video_st->codecpar->extradata_size);
        pCodecCtx->extradata_size = video_st->codecpar->extradata_size;
    }
    pCodecCtx->get_format = get_qsv_hw_format;

    ///打开解码器
    if ((ret = avcodec_open2(pCodecCtx, nullptr, nullptr)) < 0) {
        av_strerror(ret, errorbuf, sizeof(errorbuf));
        errorMsg = errorbuf;
        thread()->sigError(errorMsg);
        goto  END;
    }

    pFrame = av_frame_alloc();
    swFrame = av_frame_alloc();
    av_dump_format(pFormatCtx, 0, url.toUtf8().data(), 0); //输出视频信息

    while (!thread()->isInterruptionRequested())
    {
        if ((ret = av_read_frame(pFormatCtx, &packet)) < 0)
        {
            break; //这里认为视频读取完了
        }

        if (packet.stream_index == video_st->index) {
            ret = decode_packet(pCodecCtx, pFrame, swFrame, &packet);
        }

        qint64 end_pt = QDateTime::currentMSecsSinceEpoch();
        if((end_pt - start_pt_) >= 1000)
        {
            if(decode_frames_ != curFps_)
            {
                curFps_ = decode_frames_;
                thread()->sigCurFpsChanged(curFps_);
            }
            start_pt_ = end_pt;
            decode_frames_ = 0;
        }
        decode_frames_++;
        av_packet_unref(&packet);
    }
    //flash decoder
    packet.data = nullptr;
    packet.size = 0;
//    ret = decode_packet(pCodecCtx, pFrame, swFrame, &packet);

    thread()->sigCurFpsChanged(0);
    if(!thread()->isInterruptionRequested()){
        if(url.left(4) == "rtsp"){
            thread()->sigError("AVERROR_EOF");
        }
    }
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
}

int FFmpegQsvDecode::decode_packet(AVCodecContext *decoder_ctx, AVFrame *frame, AVFrame *sw_frame, AVPacket *pkt)
{
    int ret = 0;
    QString errorMsg;
    char errorbuf[1024]{0};

    ret = avcodec_send_packet(decoder_ctx, pkt);
    if (ret < 0) {
        av_strerror(ret, errorbuf, sizeof(errorbuf));
        errorMsg = errorbuf;
        // thread()->sigError(errorMsg);
        return ret;
    }

    while (ret >= 0) {
        int i, j;

        ret = avcodec_receive_frame(decoder_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            av_strerror(ret, errorbuf, sizeof(errorbuf));
            errorMsg = QString("Error during decoding: %1").arg(errorbuf);
            thread()->sigError(errorMsg);
            return ret;
        }

        //gpu拷贝到cpu，相对耗时
        ret = av_hwframe_transfer_data(sw_frame, frame, 0);
        if (ret < 0) {
            av_strerror(ret, errorbuf, sizeof(errorbuf));
            errorMsg = QString("Error transferring the data to system memory: %1").arg(errorbuf);
            thread()->sigError(errorMsg);
            goto fail;
        }
//        sw_frame->pts =  frame->best_effort_timestamp;
//        if(sw_frame->pts != AV_NOPTS_VALUE)
//        {
//            if(last_pts_ != AV_NOPTS_VALUE)
//            {
//                AVRational ra;
//                ra.num = 1;
//                ra.den = AV_TIME_BASE;
//                int64_t delay = av_rescale_q(sw_frame->pts - last_pts_, stream_time_base_, ra);
//                if(delay > 0 && delay < 1000000)
//                {
//                    QThread::usleep(delay);
//                }
//            }
//            last_pts_ = sw_frame->pts;
//        }
        if(!buffer_)
        {
            bufferSize_ = av_image_get_buffer_size(AVPixelFormat(sw_frame->format), sw_frame->width,
                                                   sw_frame->height, 1);

            buffer_ = (uint8_t*) av_malloc(bufferSize_);
        }
        ret = av_image_copy_to_buffer(buffer_, bufferSize_,
                                      (const uint8_t * const *)sw_frame->data,
                                      (const int *)sw_frame->linesize, AVPixelFormat(sw_frame->format),
                                      sw_frame->width, sw_frame->height, 1);

        thread()->Render([&](){
            if(!render_)
            {
                render_ = thread()->getRender(sw_frame->format);
                render_->initialize(sw_frame->width, sw_frame->height);
                thread()->sigVideoStarted(sw_frame->width, sw_frame->height);
            }
            render_->upLoad(buffer_, sw_frame->width, sw_frame->height);
        });

    fail:
        av_frame_unref(sw_frame);
        av_frame_unref(frame);

        if (ret < 0)
            return ret;
    }

    return  0;
}
