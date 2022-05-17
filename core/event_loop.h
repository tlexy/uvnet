#ifndef UVCORE_EVENT_LOOP_H
#define UVCORE_EVENT_LOOP_H

#include "../utils/ns_helper.h"
#include <thread>
#include <string>
#include <memory>
#include <uv.h>
#include <map>
#include <functional>
#include "async_event.h"

NS_UVCORE_B

using AsyncCallback = std::function<void()>;

//class AsyncEvent;

class EventLoop
{
public:
	EventLoop();

	uv_loop_t* uv_loop();
	int run();

	//在另一个线程里停止当前loop;
	void stop();

	void runInLoop(const AsyncCallback cb);
	bool isRunInLoopThread();

	virtual ~EventLoop();

protected:
	uv_loop_t* _loop{ NULL };
	AsyncEvent* _async_event;
	std::thread::id loopThreadId_{};
};

NS_UVCORE_E

#endif