#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QOpenGLWidget>
#include <mutex>
#include <QTimer>
#include "render/videorender.h"

QT_FORWARD_DECLARE_CLASS(DecodeTask)
class VideoWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    enum PlayState{
        Stop,
        Reading,
        Playing
    };
    VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget() override;
    PlayState playState() const;
    void startPlay(QString url,QString decodeName);
    int videoWidth() const;
    int videoHeidht() const;
    int fps() const;
    int curFps() const;
    QString url() const;
    QString decoderName() const;

public slots:
    void stop();

signals:
    void sigError(QString);
    void sigCurFpsChanged(int);
    void sigVideoStarted(int,int);
    void sigVideoStoped();

protected:
    class Render{
    public:
        virtual ~Render(){};
        virtual void initsize(QOpenGLContext *ctx) = 0;
        virtual void render(QOpenGLContext *ctx) = 0;
    };
    virtual Render* createRender() const{return nullptr;}
    void initializeGL() override;
    void paintGL() override;

private:
    PlayState m_state = Stop;
    VideoRenderManager *m_renderM{nullptr};
    AVPixelFormat m_fmt;
    int m_videoW,m_videoH;
    DecodeTask *m_decoThr{nullptr};
    QString m_url,m_decoderName;
    Render *render_{nullptr};
    uchar *buffer_ = nullptr;

private slots:
    void slotVideoStarted(uchar*,int,int,int);
};

#endif // VIDEOWIDGET_H
