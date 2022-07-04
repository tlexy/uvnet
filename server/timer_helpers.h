#ifndef NODE_MGR_TIMER_HELPERS_H
#define NODE_MGR_TIMER_HELPERS_H

#include "../utils/ns_helper.h"
#include <memory>
#include <core/timer.h>
#include <stdint.h>

NS_UVCORE_B

class TimerHelpers
{
public:
	//只有loop启动后才能调用
	static std::shared_ptr<uvcore::Timer> add_uv_timer(EventLoop* loop, uint64_t timeout, uint64_t repeat, TimerCallback callback);

	//called before loop start
	static std::shared_ptr<uvcore::Timer> add_uv_timer2(EventLoop* loop, uint64_t timeout, uint64_t repeat, TimerCallback callback);
};

NS_UVCORE_E

#endif