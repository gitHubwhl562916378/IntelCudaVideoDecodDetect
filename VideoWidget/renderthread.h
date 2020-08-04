#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "render/factory.h"
#include "render/videorender.h"
QT_FORWARD_DECLARE_CLASS(VideoWidget)
QT_FORWARD_DECLARE_CLASS(QOpenGLContext)
class RenderThread : public QThread
{
    Q_OBJECT
public:
    RenderThread(VideoWidget *glw, QObject *parent = nullptr);
    ~RenderThread() override;
    void lockRenderer() { m_renderMutex.lock(); }
    void unlockRenderer() { m_renderMutex.unlock(); }
    QMutex *grabMutex() { return &m_grabMutex; }
    QWaitCondition *grabCond() { return &m_grabCond; }
    void prepareExit() {m_grabCond.wakeAll(); }

    virtual VideoRender* getRender(int);
    virtual void Render(const std::function<void (void)>);

    void setFileName(QString);
    void setDevice(QString);

signals:
    void sigContextWanted();

    void sigError(QString);
    void sigVideoStarted(int, int);
    void sigFps(int);
    void sigCurFpsChanged(int);

protected:
    void run() override;

private:
    static Factory<VideoRender, int>* renderFactoryInstance();
    static std::atomic_bool isInited_;

    VideoWidget *gl_widget_;
    QMutex m_renderMutex;
    QMutex m_grabMutex;
    QWaitCondition m_grabCond;

    QString file_name_, device_;
};

#endif // RENDERTHREAD_H
