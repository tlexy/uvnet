#include "thread_timer_eventloop.h"

NS_UVCORE_B

ThreadTimerEventLoop::ThreadTimerEventLoop()
{}

void ThreadTimerEventLoop::init(int millis, TimerEVCallback cb)
{
	_tel = std::make_shared<TimerEventLoop>(millis, cb);
}

void ThreadTimerEventLoop::stop_thread()
{
	_tel->stop();
	PaThread::stop_thread();
}

void ThreadTimerEventLoop::run()
{
	_tel->start_timer();
	_tel->run();
}

std::shared_ptr<EventLoop> ThreadTimerEventLoop::get_loop()
{
	return _tel;
}

NS_UVCORE_E