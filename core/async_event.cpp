#include "async_event.h"
#include <iostream>

NS_UVCORE_B

AsyncEvent::AsyncEvent(uv_loop_t* loop)
	:_loop(loop)
{
	
}

void AsyncEvent::init()
{
    _async = (uv_async_t*)malloc(sizeof(uv_async_t));
    uv_async_init(_loop, _async, AsyncEvent::Callback);
    _async->data = this;
}

void AsyncEvent::runInLoop(const AsyncCallback cb)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _cbs.push(cb);
    }

    uv_async_send(_async);
}

void AsyncEvent::process()
{
    std::queue<AsyncCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _cbs.swap(callbacks);
    }
    while (!callbacks.empty())
    {
        auto func = callbacks.front();
        func();
        callbacks.pop();
    }
}

void AsyncEvent::Callback(uv_async_t* handle)
{
    auto async = static_cast<AsyncEvent*>(handle->data);
    async->process();
}

void AsyncEvent::close()
{
    if (uv_is_closing((uv_handle_t*)_async) == 0)
    {
        uv_close((uv_handle_t*)_async, [](uv_handle_t* handle) {
            //free(handle);
            std::cout << "async event is close" << std::endl;
        });
    }
}

AsyncEvent::~AsyncEvent()
{
    if (_async)
    {
        free(_async);
        _async = NULL;
    }
}

NS_UVCORE_E