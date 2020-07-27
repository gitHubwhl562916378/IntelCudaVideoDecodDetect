#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QDateTime>
#include "videowidget.h"
#include "render/rendermanager.h"
#include "videodecode/decodtask.h"
#include <QDebug>

VideoWidget::VideoWidget(QWidget *parent):
    QOpenGLWidget(parent)
{
    m_renderM = new RenderManager;
}

VideoWidget::~VideoWidget()
{
    stop();
    makeCurrent();
    delete m_renderM;
    if(render_){
        delete render_;
    }
}

VideoWidget::PlayState VideoWidget::playState() const
{
    return m_state;
}

void VideoWidget::startPlay(QString url, QString decodeName)
{
    m_state = Reading;
    if(m_decoThr)
    {
        stop();
    }

    m_decoThr = DecodeTaskManager::Instance()->makeTask(decodeName);
    if(!m_decoThr)
    {
        emit sigError("No such device to decode video");
        return;
    }
    m_decoderName = decodeName;
    m_url = url;
    connect(m_decoThr,SIGNAL(sigVideoStarted(uchar*,int,int,int)),this,SLOT(slotVideoStarted(uchar*,int,int,int)));
    connect(m_decoThr,SIGNAL(sigCurFpsChanged(int)), this, SIGNAL(sigCurFpsChanged(int)));
    connect(m_decoThr,SIGNAL(sigFrameLoaded()),this,SLOT(update()));
//    connect(m_decoThr, &DecodeTask::sigFrameLoaded,this, [&]{update();});
    connect(m_decoThr,SIGNAL(sigError(QString)),this,SIGNAL(sigError(QString)));
    connect(m_decoThr,SIGNAL(finished()),this,SLOT(stop()));
    m_decoThr->startPlay(url);
}

int VideoWidget::videoWidth() const
{
    return m_videoW;
}

int VideoWidget::videoHeidht() const
{
    return m_videoH;
}

int VideoWidget::fps() const
{
    if(m_decoThr)
    {
        return  m_decoThr->fps();
    }

    return 0;
}

int VideoWidget::curFps() const
{
    if(m_decoThr)
    {
        return m_decoThr->curFps();
    }

    return 0;
}

QString VideoWidget::url() const
{
    return m_url;
}

QString VideoWidget::decoderName() const
{
    return m_decoderName;
}

void VideoWidget::stop()
{
    m_state = Stop;
    if(m_decoThr)
    {
        buffer_ = nullptr;
        m_decoThr->stop();
        m_decoThr->disconnect(this);
        delete m_decoThr;
        m_decoThr = nullptr;
    }
    update();
    emit sigVideoStoped();
}

void VideoWidget::initializeGL()
{
    m_renderM->registerRender(AV_PIX_FMT_NV12);
    m_renderM->registerRender(AV_PIX_FMT_YUV420P);
    m_renderM->registerRender(AV_PIX_FMT_YUVJ420P);
    render_ = createRender();
    if(render_){
        render_->initsize(QOpenGLContext::currentContext());
    }
}

void VideoWidget::paintGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(!m_decoThr)return;
    m_renderM->render(m_fmt, buffer_, m_videoW, m_videoH);

    if(render_){
        render_->render(QOpenGLContext::currentContext());
    }
}

void VideoWidget::slotVideoStarted(uchar* ptr,int pixformat,int width,int height)
{
    m_videoW = width;
    m_videoH = height;
    buffer_ = ptr;
    m_state = Playing;
    m_fmt = AVPixelFormat(pixformat);
    emit sigVideoStarted(width, height);
}
