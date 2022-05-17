#include "timer.h"
#include "event_loop.h"
#include <iostream>

NS_UVCORE_B

NGenerator<int64_t> Timer::_gentor;

Timer::Timer(EventLoop* loop, uint64_t timeout, uint64_t repeat, TimerCallback callback)
	:_timeout_cb(callback),
	_timeout(timeout),
	_repeat(repeat),
	_loop(loop),
	_id(_gentor.get_next())
{
	
}

int64_t Timer::id()
{
	return _id;
}

void Timer::pre_start()
{
	do_start();
}

void Timer::start()
{
	if (_loop->isRunInLoopThread())
	{
		do_start();
	}
	else
	{
		_loop->runInLoop(std::bind(&Timer::do_start, this));
	}
}

void Timer::do_start()
{
	if (_is_start)
	{
		return;
	}
	_handle = (uv_timer_t*)malloc(sizeof(uv_timer_t));
	if (_handle != NULL)
	{
		_is_start = true;
		_handle->data = this;
		uv_timer_init(_loop->uv_loop(), _handle);
		uv_timer_start(_handle, Timer::do_timer, _timeout, _repeat);
	}
}

void Timer::reset_repeat(int64_t ms)
{
	_repeat = ms;
	::uv_timer_set_repeat(_handle, ms);
}

void Timer::on_close()
{
	if (_close_cb)
	{
		_close_cb(this);
	}
}

void Timer::do_close()
{
	if (uv_is_active((uv_handle_t*)_handle))
	{
		uv_timer_stop(_handle);
	}
	if (uv_is_closing((uv_handle_t*)_handle) == 0)
	{
		_gentor.recyle(_id);
		::uv_close((uv_handle_t*)_handle,
			[](uv_handle_t* handle)
			{
				auto ptr = static_cast<Timer*>(handle->data);
				free(handle);
				ptr->on_close();
			});
	}
}

void Timer::close(TimerCallback cb)
{
	_close_cb = cb;
	if (_loop->isRunInLoopThread())
	{
		do_close();
	}
	else
	{
		_loop->runInLoop(std::bind(&Timer::do_close, this));
	}
	
}

void Timer::on_timeout()
{
	//std::cout << "on timeout, id:" << id() << std::endl;
	if (_timeout_cb)
	{
		_timeout_cb(this);
	}
}

void Timer::do_timer(uv_timer_t* handle)
{
	auto ptr = static_cast<Timer*>(handle->data);
	ptr->on_timeout();
}

NS_UVCORE_E