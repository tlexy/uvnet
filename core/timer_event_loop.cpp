#include "timer_event_loop.h"
#include "timer.h"
#include <iostream>

NS_UVCORE_B

TimerEventLoop::TimerEventLoop(int64_t mills, TimerEVCallback cb)
	:_mills(mills),
	_cb(cb)
{

}

void TimerEventLoop::timer_event(Timer*)
{
	if (_cb)
	{
		_cb();
	}
	//std::cout << "TimerEventLoop timer tick." << std::endl;
}

void TimerEventLoop::start_timer()
{
	_timer = std::make_shared<Timer>(this, _mills, _mills, std::bind(&TimerEventLoop::timer_event, this, std::placeholders::_1));
	_timer->pre_start();
}

TimerEventLoop::~TimerEventLoop()
{

}

NS_UVCORE_E