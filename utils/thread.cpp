#include "thread.h"


PaThread::PaThread()
    :_stop(true),
    _thread(std::shared_ptr<std::thread>())
{
}

void PaThread::stop_thread()
{
    if (!_stop && _thread)
    {
        _stop = true;
        if (_thread->joinable())
        {
            _thread->join();
            _thread = std::shared_ptr<std::thread>();
        }
    }
}

void PaThread::start_thread()
{
    _stop = false;
    _thread = std::make_shared<std::thread>([this]()
        {
            run();
        });
}

bool PaThread::is_stop() const
{
    return _stop;
}
