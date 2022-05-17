#include "tcp_connection.h"
#include "../utils/global.h"
#include <thread>
#include "event_loop.h"
#include <iostream>
#include "../utils/sutil.h"
#include "../utils/byte_order.hpp"
#ifndef _WIN32
#include <jemalloc/jemalloc.h>
#endif

NS_UVCORE_B

NGenerator<int64_t> TcpConnection::_gentor;

TcpConnection::TcpConnection(std::shared_ptr<EventLoop> loop, uv_tcp_t* handle, bool del)
	:_handle(handle),
	_connid(_gentor.get_next()),
	_loop_ptr(loop),
	_buffer(Global::get_instance()->socket_buffer()),
	_cb(DataCallBack()),
	_ccb(CloseCallBack()),
	_handle_del(del)
{
	_create_time = SUtil::getTimeStampMilli();
}

void TcpConnection::calc_ip()
{
	//IP地址相关
	struct sockaddr_in sock_addr_remote;
	int sock_len = sizeof(struct sockaddr_in);
	uv_tcp_getpeername(_handle, (struct sockaddr*)&sock_addr_remote, &sock_len);
	_remote_ip = sock_addr_remote.sin_addr.s_addr;
	_remote_ip = sockets::networkToHost32(_remote_ip);
}

TcpConnection::~TcpConnection()
{
	//std::cout << "TcpConnection dtor, id: " << id() << std::endl;
}

uint32_t TcpConnection::get_remote_ip()
{
	return _remote_ip;
}

void TcpConnection::set_error(int err)
{
	_error = err;
}

int TcpConnection::error()
{
	return _error;
}

void TcpConnection::set_state(int st)
{
	_state = st;
}

int TcpConnection::state()
{
	return _state;
}

void TcpConnection::set_create_time(int64_t ct)
{
	_create_time = ct;
}

int64_t TcpConnection::create_time()
{
	return _create_time;
}

char* TcpConnection::get_buffer()
{
	return (char*)_buffer.enable_size(Global::get_instance()->packet_size());
}

uint32_t TcpConnection::get_buffer_length()
{
	return Global::get_instance()->packet_size();
}

CircleBuffer* TcpConnection::get_inner_buffer()
{
	return &_buffer;
}

void TcpConnection::has_written(size_t len)
{
	_buffer.has_written(len);
}

void TcpConnection::on_receive_data(size_t len)
{
	if (_is_close)
	{
		return;
	}
	
	//std::cout << "on_receive_data, cid: " << id() << std::endl;
	if (_cb)
	{
		_cb(shared_from_this());
	}
}

void TcpConnection::set_receive_cb(DataCallBack cb)
{
	_cb = cb;
}

void TcpConnection::set_close_cb(CloseCallBack cb)
{
	_ccb = cb;
}

int TcpConnection::write(const char* data, int len)
{
	if (len <= 0)
	{
		return -1;
	}
	if (_is_close || _error  != 0)
	{
		return 3;
	}
	if (!_loop_ptr->isRunInLoopThread())
	{
		return 1;
	}
	WriteReq* req = new WriteReq;
	req->req.data = this;
	char* buf = (char*)malloc(len);
	std::copy(data, data + len, buf);
	if (buf == NULL)
	{
		return 2;//内存分配失败
	}
	req->buf = uv_buf_init(const_cast<char*>(buf), static_cast<unsigned int>(len));
	uv_write((uv_write_t*)req, (uv_stream_t*)_handle, &req->buf, 1, TcpConnection::write_cb);
	return 0;
}

int TcpConnection::async_write(WriteReq* req)
{
	if (!_is_close && _error == 0)
	{
		uv_write((uv_write_t*)req, (uv_stream_t*)_handle, &req->buf, 1, TcpConnection::write_cb);
	}
	return 0;
}

int TcpConnection::writeInLoop(const char* data, int len)
{
	if (len <= 0)
	{
		return -1;
	}
	if (_is_close)
	{
		return 3;
	}
	if (_loop_ptr->isRunInLoopThread())
	{
		return write(data, len);
	}
	else
	{
		WriteReq* req = new WriteReq;
		req->req.data = this;
		char* buf = (char*)malloc(len);
		if (buf == NULL)
		{
			return 2;//内存分配失败
		}
		std::copy(data, data + len, buf);
		req->buf = uv_buf_init(const_cast<char*>(buf), static_cast<unsigned int>(len));
		_loop_ptr->runInLoop(std::bind(&TcpConnection::async_write, this, req));
	}
	return 0;
}

void TcpConnection::write_cb(uv_write_t* preq, int status)
{
	WriteReq* ireq = (WriteReq*)preq;
	free(ireq->buf.base);

	TcpConnection* conn = (TcpConnection*)ireq->req.data;
	if (conn && conn->_del_after_write)
	{
		conn->close();
	}
	delete ireq;
}

void TcpConnection::del_after_write()
{
	_del_after_write = true;
}

void TcpConnection::on_close()
{
	if (_ccb)
	{
		_ccb(shared_from_this());
	}
	_gentor.recyle(id());
}

bool TcpConnection::del_handle() const
{
	return _handle_del;
}

void TcpConnection::close()
{
	if (_is_close)
	{
		return;
	}
	
	if (_loop_ptr->isRunInLoopThread())
	{
		_is_close = true;
		if (uv_is_active((uv_handle_t*)_handle))
		{
			uv_read_stop((uv_stream_t*)_handle);
		}
		if (uv_is_closing((uv_handle_t*)_handle) == 0)
		{
			uv_close((uv_handle_t*)_handle, [](uv_handle_t* handle) {
				TcpConnection* conn = (TcpConnection*)handle->data;
				if (conn) {
					if (conn->del_handle())
					{
						free(handle);
					}
					conn->on_close();
				}
			});
		}
	}
	else
	{
		_loop_ptr->runInLoop(std::bind(&TcpConnection::close, this));
	}
}

NS_UVCORE_E