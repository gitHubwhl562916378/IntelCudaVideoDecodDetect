#ifndef DECODTASK_H
#define DECODTASK_H

#include <QThread>

class DecodeTask : public QThread
{
    Q_OBJECT
public:
    DecodeTask(QObject* parent = nullptr):QThread(parent){}
    ~DecodeTask(){}
    virtual void startPlay(QString) = 0;
    virtual int fps() = 0;
    virtual int curFps() = 0;
    virtual void stop() = 0;

signals:
    void sigVideoStarted(uchar*,int,int,int);
    void sigCurFpsChanged(int);
    void sigFrameLoaded();
    void sigError(QString);
};

class DecodeTaskManager
{
public:
    DecodeTaskManager() = default;
    virtual ~DecodeTaskManager(){}
    virtual DecodeTask* makeTask(const QString) = 0;

    static DecodeTaskManager* Instance();
};

#endif // DECODTASK_H
