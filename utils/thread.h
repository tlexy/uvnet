#ifndef PACTOR_THREAD_H
#define PACTOR_THREAD_H

#include <thread>
#include <memory>

class PaThread
{
public:
    PaThread();

    void start_thread();
    virtual void stop_thread();
    bool is_stop() const;
    

protected:
    bool _stop;
    std::shared_ptr<std::thread> _thread;

protected:
    virtual void run() = 0;
};

#endif

