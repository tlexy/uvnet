#ifndef UVCORE_ASYNC_EVENT_H
#define UVCORE_ASYNC_EVENT_H

#include "../utils/ns_helper.h"
#include <uv.h>
#include <map>
#include <functional>
#include <queue>
#include <mutex>

NS_UVCORE_B

using AsyncCallback = std::function<void()>;

class AsyncEvent
{
public:
	AsyncEvent(uv_loop_t*);

	void init();

	void runInLoop(const AsyncCallback cb);

	void close();

	~AsyncEvent();

private:
	void process();
	static void Callback(uv_async_t* handle);

private:
	uv_loop_t* _loop{ NULL };
	uv_async_t* _async{NULL};
	std::mutex _mutex;
	std::queue<AsyncCallback> _cbs;
};

NS_UVCORE_E

#endif