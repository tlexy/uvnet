#include "tcp_client.h"
#include "tcp_connection.h"
#include "event_loop.h"
#include <iostream>
#include "timer.h"
#include "../server/timer_helpers.h"

NS_UVCORE_B

extern "C"
{

	void client_alloc_buffer(uv_handle_t* handle,
		size_t suggested_size,
		uv_buf_t* buf)
	{
		TcpConnection* connection = (TcpConnection*)handle->data;
		buf->base = (char*)connection->get_buffer();
		buf->len = connection->get_buffer_length();
	}

	void client_after_read(uv_stream_t* stream,
		ssize_t nread,
		const uv_buf_t* buf)
	{
		TcpConnection* connection = (TcpConnection*)stream->data;
		if (nread > 0)
		{
			connection->has_written(nread);
			connection->on_receive_data(nread);
		}
		else if (nread < 0)
		{
			//std::cout << "tcpclient connection lose" << std::endl;
			//connection->close();
			connection->set_error(1);
			connection->on_receive_data(0);
		}
		else
		{
			//Note nread might be 0, which does not indicate an error or EOF. This is equivalent to EAGAIN or EWOULDBLOCK under read(2).
		}
	}

	void client_close_cb(uv_handle_t* handle)
	{
/*		TcpClient* tcp = nullptr;
		if (handle->data != NULL)
		{
			tcp = static_cast<TcpClient*>(handle->data);
			free(handle);
		}
		if (tcp)
		{
			tcp->on_close();
		}*/
	}
}

TcpClient::TcpClient(std::shared_ptr<EventLoop> loop, const IpAddress& addr)
	:_loop_ptr(loop),
	_client_addr(addr)
{
	
}

TcpClient::~TcpClient()
{
	if (_client_handle)
	{
		//free(_client_handle);
		do_close();
		_client_handle = NULL;
	}
}

void TcpClient::close()
{
	if (_loop_ptr->isRunInLoopThread())
	{
		do_close();
	}
	else
	{
		_loop_ptr->runInLoop(std::bind(&TcpClient::do_close, this));
	}
}

void TcpClient::close_connection()
{
	if (_loop_ptr->isRunInLoopThread())
	{
		do_close_connection();
	}
	else
	{
		_loop_ptr->runInLoop(std::bind(&TcpClient::do_close_connection, this));
	}
}

void TcpClient::do_close_connection()
{
	_conn_ptr->close();
}

void TcpClient::do_close()
{
	_conn_ptr->close();
	//_client_handle会在tcp_connection进行uv_close,但不会释放内存
	//uv_close((uv_handle_t*)_client_handle, client_close_cb);
	//_client_handle = NULL;
	if (_connect_timer)
	{
		_connect_timer->close(nullptr);
	}
}

void TcpClient::set_connect_timeout(int milli)
{
	//超时时间到后，删除对象，返回连接失败
	_connect_timeout = milli;
}

void TcpClient::async_connect(HandshadeCompleteCallBack cb)
{
	_cb = cb;
	
	if (_loop_ptr->isRunInLoopThread())
	{
		do_connect();
	}
	else
	{
		_loop_ptr->runInLoop(std::bind(&TcpClient::do_connect, this));
	}
}

void TcpClient::pre_connect(HandshadeCompleteCallBack cb)
{
	_cb = cb;
	do_connect();
}

void TcpClient::on_connect_timer(uvcore::Timer*)
{
	_connect_timer->stop();
	if (_is_on_connecting == true)
	{
		//loge("connect to server [%s] timeout", _client_addr.toString().c_str());
		close_connection();
		//连接超时
		/*if (_cb)
		{
			_cb(-10086, nullptr);
		}*/
	}
}

int TcpClient::init()
{
	if (_client_handle)
	{
		free(_client_handle);
	}
	_client_handle = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	_conn_ptr = std::make_shared<TcpConnection>(_loop_ptr, _client_handle, false);

	int r = uv_tcp_init(_loop_ptr->uv_loop(), _client_handle);
	uv_tcp_nodelay(_client_handle, 1);
	_client_handle->data = NULL;
	_client_connect.data = static_cast<void*>(this);
	return r;
}

void TcpClient::do_connect()
{
	init();

	_client_addr.update_addr();
	struct sockaddr_in addr;
	uv_ip4_addr(_client_addr.getIp().c_str(), _client_addr.getPort(), &addr);

	_client_handle->data = _conn_ptr.get();
	_conn_ptr->set_close_cb(std::bind(&TcpClient::on_connection_close_, this, std::placeholders::_1));
	
	_is_on_connecting = true;
	uv_tcp_connect(&_client_connect, _client_handle, (const struct sockaddr*)&addr, [](uv_connect_t* req, int status) {

		auto handle = static_cast<TcpClient*>((req->data));
		handle->onConnect(status);
		});

	if (_connect_timeout > 0)
	{
		if (!_connect_timer)
		{
			int64_t now = 0;// SUtil::getTimeStampMilli();
			//_connect_timer = std::make_shared<uvcore::Timer>(get_loop().get(), now + _connect_timeout, 0, std::bind(&TcpClient::on_connect_timer, this, std::placeholders::_1));
			_connect_timer = uvcore::TimerHelpers::add_uv_timer2(get_loop().get(), now + _connect_timeout, _connect_timeout, std::bind(&TcpClient::on_connect_timer, this, std::placeholders::_1));
		}
		else
		{
			_connect_timer->restart();
		}
	}
}

std::shared_ptr<TcpConnection> TcpClient::get_connection()
{
	return _conn_ptr;
}

std::shared_ptr<uvcore::EventLoop> TcpClient::get_loop()
{
	return _loop_ptr;
}

uvcore::IpAddress TcpClient::addr()
{
	return _client_addr;
}

void TcpClient::onConnect(int status)
{
	_is_on_connecting = false;
	if (_connect_timer)
	{
		_connect_timer->stop();
	}
	if (!_cb)
	{
		return;
	}

	if (status == 0)
	{	
		_conn_ptr->set_receive_cb(std::bind(&TcpClient::on_message_, this, std::placeholders::_1));

		int r = uv_read_start((uv_stream_t*)_client_handle, client_alloc_buffer, client_after_read);
		_cb(status, _conn_ptr);
	}
	else
	{
		_cb(status, _conn_ptr);
	}
}

void TcpClient::on_message_(std::shared_ptr<uvcore::TcpConnection> ptr)
{
	on_message(ptr);
}

void TcpClient::on_connection_close_(std::shared_ptr<uvcore::TcpConnection> ptr)
{
	on_connection_close(ptr);
}

NS_UVCORE_E