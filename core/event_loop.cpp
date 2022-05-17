#include "event_loop.h"
#include <iostream>

NS_UVCORE_B

EventLoop::EventLoop()
{
	_loop = new uv_loop_t();
	::uv_loop_init(_loop);
}

uv_loop_t* EventLoop::uv_loop()
{
	return _loop;
}

int EventLoop::run()
{
	_async_event = new AsyncEvent(_loop);
	_async_event->init();

	loopThreadId_ = std::this_thread::get_id();
	int ret = uv_run(_loop, UV_RUN_DEFAULT);
	std::cout << "eventloop stop, ret = " << ret << std::endl;
	uv_loop_close(_loop);
	return ret;
}

EventLoop::~EventLoop()
{
	if (_loop)
	{
		free(_loop);
		_loop = NULL;
	}
	if (_async_event)
	{
		delete _async_event;
		_async_event = NULL;
	}
}

void EventLoop::stop()
{
	if (_loop)
	{
		std::cout << "stop loop" << std::endl;
		uv_stop(_loop);
		_async_event->runInLoop([this]() {
			//下面的语句不一定能执行
			if (_async_event)
			{
				_async_event->close();
			}
		});
	}
}

void EventLoop::runInLoop(const AsyncCallback cb)
{
	_async_event->runInLoop(cb);
}

bool EventLoop::isRunInLoopThread()
{
	return loopThreadId_ == std::this_thread::get_id();
}

NS_UVCORE_E