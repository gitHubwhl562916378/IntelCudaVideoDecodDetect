#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <atomic>
#include <QOpenGLWidget>
QT_FORWARD_DECLARE_CLASS(RenderThread)
class VideoWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    enum PlayState{
        Ready,
        Play,
        Stopped
    };
    VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget() override;
    bool isFrameSwapped() const;
    QString url() const;
    QString deviceName() const;
    PlayState state() const;

public slots:
    void slotPlay(QString filename, QString device);
    void slotStop();

signals:
    void sigRequestRender();

    void sigError(QString);
    void sigVideoStarted(int, int);
    void sigFps(int);
    void sigCurFpsChanged(int);

protected:
    void paintEvent(QPaintEvent *) override { }
    void resizeGL(int w, int h) override;

private slots:
    void slotGrabContext();
    void slotAboutToCompose();
    void slotFrameSwapped();
    void slotAboutToResize();
    void slotResized();

private:
    RenderThread *m_thread;
    std::atomic_bool isFrameSwapped_;
    QString source_file_, device_name_;
    PlayState m_state_;
};

#endif // VIDEOWIDGET_H
