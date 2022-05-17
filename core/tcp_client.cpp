#include "tcp_client.h"
#include "tcp_connection.h"
#include "event_loop.h"
#include <iostream>

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
		free(_client_handle);
		_client_handle = NULL;
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

void TcpClient::async_connect(HandshadeCompleteCallBack cb)
{
	_cb = cb;
	
	if (_loop_ptr->isRunInLoopThread())
	{
		do_connect_();
	}
	else
	{
		_loop_ptr->runInLoop(std::bind(&TcpClient::do_connect_, this));
	}
}

void TcpClient::pre_connect(HandshadeCompleteCallBack cb)
{
	_cb = cb;
	do_connect_();
}

void TcpClient::do_connect_()
{
	init();
	_client_addr.update_addr();
	struct sockaddr_in addr;
	uv_ip4_addr(_client_addr.getIp().c_str(), _client_addr.getPort(), &addr);

	_client_handle->data = _conn_ptr.get();
	_conn_ptr->set_close_cb(std::bind(&TcpClient::on_connection_close_, this, std::placeholders::_1));
	
	uv_tcp_connect(&_client_connect, _client_handle, (const struct sockaddr*)&addr, [](uv_connect_t* req, int status) {

		auto handle = static_cast<TcpClient*>((req->data));
		handle->onConnect(status);
		});
}

std::shared_ptr<TcpConnection> TcpClient::get_connection()
{
	return _conn_ptr;
}

std::shared_ptr<uvcore::EventLoop> TcpClient::get_loop()
{
	return _loop_ptr;
}

void TcpClient::onConnect(int status)
{
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