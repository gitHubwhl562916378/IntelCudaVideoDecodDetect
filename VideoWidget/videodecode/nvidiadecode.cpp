/*
 * @Author: your name
 * @Date: 2020-08-03 18:38:52
 * @LastEditTime: 2020-08-03 21:05:29
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \qt_project\VideoWidget\VideoWidget\videodecode\nvidiadecode.cpp
 */
extern "C"
{
#include "libavformat/avformat.h"
}
#include <QDateTime>
#include <QDebug>
#include "../renderthread.h"
#include "nvidiadecode.h"

NvidiaDecode::NvidiaDecode(DecodeTaskManagerImpl *taskManger, CreateDecoderFunc func, RenderThread *render_thr):
    DecodeTask(render_thr),
    taskManager_(taskManger),
    createDecoderFunc_(func)
{

}

NvidiaDecode::~NvidiaDecode()
{
    qDebug() << "NvidiaDecode::~NvidiaDecode()";
    if(decoder_){
        delete decoder_;
        decoder_ = nullptr;
    }
}

extern CreateDecoderFunc g_gpurender_fuc_;
void NvidiaDecode::decode(const QString &url)
{
    if(!createDecoderFunc_){
        return;
    }

    decoder_ = createDecoderFunc_();
    if(!decoder_){
        return;
    }
    std::string error;
    decoder_->decode(url.toStdString().data(), true, [&](void* ptr, const int pix, const int width, const int height, const std::string &err){
        if(thread()->isInterruptionRequested()){
            decoder_->stop();
        }
        error = err;
        if(!err.empty())
        {
            thread()->sigError(QString::fromStdString(err));
        }
        if(pix != AV_PIX_FMT_NV12){
            return;
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

        thread()->Render([&](){
            if(!render_)
            {
                thread()->setExtraData(decoder_->context());
                render_ = thread()->getRender(AV_PIX_FMT_CUDA);
                render_->initialize(width, height);
                if(decoder_->fps())
                {
                    thread()->sigFps(decoder_->fps());
                }
                thread()->sigVideoStarted(width, height);
            }
            render_->upLoad(reinterpret_cast<unsigned char*>(ptr), width, height);
        });
    });

    if(!thread()->isInterruptionRequested()){
        if(url.left(4) == "rtsp" && error.empty()){
            thread()->sigError("AVERROR_EOF");
        }
    }
}
