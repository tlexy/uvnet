#ifndef UVCORE_TIMER_EVENT_LOOP
#define UVCORE_TIMER_EVENT_LOOP

#include "event_loop.h"
#include <chrono>

NS_UVCORE_B

using TimerEVCallback = std::function<void()>;

class Timer;

class TimerEventLoop : public EventLoop
{
public:
	TimerEventLoop(int64_t mills, TimerEVCallback cb);

	void timer_event(Timer*);

	void start_timer();

	virtual ~TimerEventLoop();

private:
	int64_t _mills;
	std::shared_ptr<Timer> _timer{};
	TimerEVCallback _cb{};
};

NS_UVCORE_E

#endif