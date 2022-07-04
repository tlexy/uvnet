#ifndef UVCORE_THREAD_TIMER_EVENT_LOOP
#define UVCORE_THREAD_TIMER_EVENT_LOOP

#include "../core/timer_event_loop.h"
#include <utils/thread.h>

NS_UVCORE_B

class ThreadTimerEventLoop : public PaThread
{
public:
	ThreadTimerEventLoop();
	std::shared_ptr<EventLoop> get_loop();
	virtual void stop_thread();
	void init(int millis, TimerEVCallback cb);

protected:
	virtual void run();

private:
	std::shared_ptr<TimerEventLoop> _tel;
};

NS_UVCORE_E

#endif