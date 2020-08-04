#ifndef DECODTASK_H
#define DECODTASK_H

#include <QString>
class RenderThread;
class DecodeTask
{
public:
    DecodeTask(RenderThread *render_thr):m_thread_(render_thr){}
    virtual ~DecodeTask(){}
    virtual RenderThread* thread(){return m_thread_;}
    virtual void decode(const QString &url) = 0;

private:
    RenderThread *m_thread_;
};

class DecodeTaskManager
{
public:
    DecodeTaskManager() = default;
    virtual ~DecodeTaskManager(){}
    virtual DecodeTask* makeTask(RenderThread*glw, const QString &device) = 0;

    static DecodeTaskManager* Instance();
};

#endif // DECODTASK_H
