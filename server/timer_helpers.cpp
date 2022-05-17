#include "timer_helpers.h"

NS_UVCORE_B

std::shared_ptr<uvcore::Timer> TimerHelpers::add_uv_timer(EventLoop* loop, uint64_t timeout, uint64_t repeat, TimerCallback callback)
{
	auto timer = std::make_shared<uvcore::Timer>(loop, timeout, repeat, callback);
	timer->start();
	return timer;
}

NS_UVCORE_E