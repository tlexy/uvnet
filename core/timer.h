#ifndef UVCORE_TIMER_H
#define UVCORE_TIMER_H

#include "../utils/ns_helper.h"
#include <uv.h>
#include <functional>
#include "../utils/ngenerator.hpp"

NS_UVCORE_B

class EventLoop;
class Timer;

using TimerCallback = std::function<void(Timer*)>;

class Timer
{
public:
	/// <summary>
	///  If repeat is non-zero, 
	///  the callback fires first after timeout milliseconds and then repeatedly after repeat milliseconds
	/// </summary>
	/// <param name="loop"></param>
	/// <param name="timeout"></param>
	/// <param name="repeat"></param>
	/// <param name="callback"></param>
	Timer(EventLoop* loop, uint64_t timeout, uint64_t repeat, TimerCallback callback);

	int64_t id();
	/*
	* 必须注意，当EventLoop未启动时， start调用可能会引起异常
	*/
	void start();
	void pre_start();
	void on_timeout();
	void reset_repeat(int64_t ms);
	void close(TimerCallback cb);

	void on_close();

private:
	void do_start();
	void do_close();

private:
	EventLoop* _loop;
	int64_t _id;
	uv_timer_t* _handle{NULL};
	uint64_t _timeout;
	uint64_t _repeat;
	bool _is_start{ false };
	TimerCallback _timeout_cb{};
	TimerCallback _close_cb{};

	static NGenerator<int64_t> _gentor;

private:
	static void do_timer(uv_timer_t* handle);
};

NS_UVCORE_E

#endif