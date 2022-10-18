#include <tls/ssl_connection.h>
#include <utils/global.h>
#include <thread>
#include <core/event_loop.h>
#include <iostream>
#include <utils/byte_order.hpp>
#include <tls/tls_config.h>
#include <string.h>

NS_UVCORE_B

SslConnection::SslConnection(std::shared_ptr<EventLoop> loop, uv_tcp_t* handle, bool is_server, bool del)
	:TcpConnection(loop, handle, del),
	_dec_buffer(Global::get_instance()->socket_buffer())
{
}

SslConnection::~SslConnection()
{
}

CircleBuffer* SslConnection::get_dec_buffer()
{
	return &_dec_buffer;
}

void SslConnection::on_receive_data(size_t len)
{
	if (_is_close)
	{
		return;
	}
	int n = 0;
	std::cerr << "recv len: " << get_inner_buffer()->readable_size() << std::endl;
	while (get_inner_buffer()->readable_size() > 0)
	{
		write_to_bio(get_inner_buffer());

		//1. TLS handshake
		if (!is_handshake_finished())
		{
			if (do_ssl_handshake() == SSLSTATUS_FAIL)
			{
				std::cerr << "SSLSTATUS_FAIL" << std::endl;
				close();
				return;
			}
			if (!is_handshake_finished())
			{
				return;
			}
			else
			{
				//可以开始通讯了，回调
				if (_ncb)
				{
					_ncb(std::dynamic_pointer_cast<SslConnection>(shared_from_this()));
				}
			}
		}

		//2. receive data to decencrypt
		read_from_bio(get_dec_buffer());

	}
	TcpConnection::on_receive_data(_dec_buffer.readable_size());
}

void SslConnection::set_new_ssl_cb(SslCallBack cb)
{
	_ncb = cb;
}

void SslConnection::write_socket(const char* data, int len)
{
	TcpConnection::write(data, len);
}

int SslConnection::write(const char* data, int len)
{
	auto status = write_to_ssl(data, len);
	if (status == SSLSTATUS_FAIL)
	{
		return -1;
	}
	//write_socket((const char*)_raw_write_buffer.read_ptr(), _raw_write_buffer.readable_size());
	int ret = TcpConnection::write((const char*)_raw_write_buffer.read_ptr(), _raw_write_buffer.readable_size());
	_raw_write_buffer.reset();
	return ret;
}

int SslConnection::writeInLoop(const char* data, int len)
{
	auto status = write_to_ssl(data, len);
	if (status == SSLSTATUS_FAIL)
	{
		return -1;
	}
	if (_loop_ptr->isRunInLoopThread())
	{
		int ret = TcpConnection::write((const char*)_raw_write_buffer.read_ptr(), _raw_write_buffer.readable_size());
		_raw_write_buffer.reset();
		return ret;
	}
	else
	{
		WriteReq* req = new WriteReq;
		req->req.data = this;
		int real_len = _raw_write_buffer.readable_size();
		char* buf = (char*)malloc(real_len);
		if (buf == NULL)
		{
			return 2;//内存分配失败
		}
		memcpy(buf, _raw_write_buffer.read_ptr(), real_len);
		_raw_write_buffer.has_read(real_len);
		req->buf = uv_buf_init(const_cast<char*>(buf), static_cast<unsigned int>(real_len));
		_loop_ptr->runInLoop(std::bind(&SslConnection::async_write, this, req));
	}
	return 0;
}

NS_UVCORE_E
