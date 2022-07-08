#include <tls/ssl_connection.h>
#include <utils/global.h>
#include <thread>
#include <core/event_loop.h>
#include <iostream>
#include <utils/byte_order.hpp>
#include <tls/tls_config.h>

NS_UVCORE_B

NGenerator<int64_t> SslConnection::_gentor;

SslConnection::SslConnection(std::shared_ptr<EventLoop> loop, uv_tcp_t* handle, bool is_server, bool del)
	:_handle(handle),
	_connid(_gentor.get_next()),
	_loop_ptr(loop),
	_buffer(Global::get_instance()->socket_buffer()),
	_dec_buffer(Global::get_instance()->socket_buffer()),
	_raw_write_buffer(Global::get_instance()->socket_buffer()),
	_cb(SslCallBack()),
	_ccb(SslCallBack()),
	_handle_del(del),
	_is_server(is_server)
{
	_create_time = getTimeStampMilli();

	_ssl = SSL_new(TlsConfig::get_ssl_ctx());
	_read_bio = BIO_new(BIO_s_mem());
	_write_bio = BIO_new(BIO_s_mem());
	if (is_server)
	{
		SSL_set_accept_state(_ssl);
	}
	else
	{
		SSL_set_connect_state(_ssl);
	}

	SSL_set_bio(_ssl, _read_bio, _write_bio);
}

void SslConnection::calc_ip()
{
	//IP地址相关
	struct sockaddr_in sock_addr_remote;
	int sock_len = sizeof(struct sockaddr_in);
	uv_tcp_getpeername(_handle, (struct sockaddr*)&sock_addr_remote, &sock_len);
	_remote_ip = sock_addr_remote.sin_addr.s_addr;
	_remote_ip = sockets::networkToHost32(_remote_ip);
}

SslConnection::~SslConnection()
{
	//std::cout << "TcpConnection dtor, id: " << id() << std::endl;
}

uint32_t SslConnection::get_remote_ip()
{
	return _remote_ip;
}

void SslConnection::set_error(int err)
{
	_error = err;
}

int SslConnection::error()
{
	return _error;
}

void SslConnection::set_state(int st)
{
	_state = st;
}

int SslConnection::state()
{
	return _state;
}

void SslConnection::set_create_time(int64_t ct)
{
	_create_time = ct;
}

int64_t SslConnection::create_time()
{
	return _create_time;
}

char* SslConnection::get_buffer()
{
	return (char*)_buffer.enable_size(Global::get_instance()->packet_size());
}

uint32_t SslConnection::get_buffer_length()
{
	return Global::get_instance()->packet_size();
}

CircleBuffer* SslConnection::get_inner_buffer()
{
	return &_buffer;
}

CircleBuffer* SslConnection::get_dec_buffer()
{
	return &_dec_buffer;
}

void SslConnection::has_written(size_t len)
{
	_buffer.has_written(len);
}

std::shared_ptr<EventLoop> SslConnection::loop()
{
	return _loop_ptr;
}

SslConnection::SSLStatus SslConnection::do_ssl_handshake()
{
	ERR_clear_error();
	int ret = SSL_do_handshake(_ssl);
	SSLStatus status = get_ssl_status(ret);

	if (status == SSLSTATUS_WANT_IO)
	{
		do {
			_raw_write_buffer.enable_size(128);
			ret = BIO_read(_write_bio, _raw_write_buffer.write_ptr(), _raw_write_buffer.writable_size());
			if (ret > 0)
			{
				_raw_write_buffer.has_written(ret);
				write_socket((const char*)_raw_write_buffer.read_ptr(), _raw_write_buffer.readable_size());
				_raw_write_buffer.reset();
			}
			else if (!BIO_should_retry(_write_bio))
			{
				return SSLSTATUS_FAIL;
			}
		} while (ret > 0);
	}
	return status;
}

SslConnection::SSLStatus SslConnection::get_ssl_status(int n) {
	int code = SSL_get_error(_ssl, n);
	std::cout << "SSL_get_error: " << code << std::endl;
	switch (code) {
		case SSL_ERROR_NONE:
			return SSLSTATUS_OK;

		case SSL_ERROR_WANT_WRITE:
		{
			std::cout << "SSL_ERROR_WANT_WRITE" << std::endl;
		}
		case SSL_ERROR_WANT_READ:
		{
			std::cout << "SSL_ERROR_WANT_READ" << std::endl;
			return SSLSTATUS_WANT_IO;
		}

		case SSL_ERROR_ZERO_RETURN:
		{
			std::cout << "SSL_ERROR_ZERO_RETURN" << std::endl;
		}
		case SSL_ERROR_SYSCALL:
		{
			std::cout << "SSL_ERROR_SYSCALL" << std::endl;
		}
		default:
		{
			std::cout << "default..." << std::endl;
			return SSLSTATUS_FAIL;
		}
	}
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
		n = BIO_write(_read_bio, get_inner_buffer()->read_ptr(), get_inner_buffer()->readable_size());
		get_inner_buffer()->has_read(n);

		if (!SSL_is_init_finished(_ssl))
		{
			if (do_ssl_handshake() == SSLSTATUS_FAIL)
			{
				std::cerr << "SSLSTATUS_FAIL" << std::endl;
				SSL_clear(_ssl);
				close();
				return;
			}
			if (!SSL_is_init_finished(_ssl))
			{
				return;
			}
			else
			{
				//可以开始通讯了，回调
				if (_ncb)
				{
					_ncb(shared_from_this());
				}
			}
		}

		do {
			_dec_buffer.enable_size(2048);
			n = SSL_read(_ssl, _dec_buffer.write_ptr(), _dec_buffer.writable_size());
			std::cerr << "decode len: " << n << std::endl;
			if (n > 0)
			{
				_dec_buffer.has_written(n);
				uint8_t* pp = _dec_buffer.read_ptr();
				int ss = _dec_buffer.readable_size();
				int a = 1;
			}
		} while (n > 0);

	}
	//std::cout << "on_receive_data, cid: " << id() << std::endl;
	if (_cb)
	{
		_cb(shared_from_this());
	}
}

void SslConnection::set_receive_cb(SslCallBack cb)
{
	_cb = cb;
}

void SslConnection::set_close_cb(SslCallBack cb)
{
	_ccb = cb;
}

void SslConnection::set_new_ssl_cb(SslCallBack cb)
{
	_ncb = cb;
}

int SslConnection::write_socket(const char* data, int len)
{
	if (len <= 0)
	{
		return -1;
	}
	if (!_loop_ptr->isRunInLoopThread())
	{
		return 1;
	}
	if (_is_close || _error != 0)
	{
		return 3;
	}
	if (uv_is_closing((uv_handle_t*)_handle) || id() < 1)
	{
		return 3;
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
	uv_write((uv_write_t*)req, (uv_stream_t*)_handle, &req->buf, 1, SslConnection::write_cb);
	return 0;
}

int SslConnection::write(const char* data, int len)
{
	int n = SSL_write(_ssl, data, len);
	SSLStatus status = get_ssl_status(n);
	if (n > 0)
	{
		do {
			_raw_write_buffer.enable_size(len);
			n = BIO_read(_write_bio, _raw_write_buffer.write_ptr(), _raw_write_buffer.writable_size());
			if (n > 0)
			{
				_raw_write_buffer.has_written(n);
			}
		} while (n > 0);
	}
	if (status == SSLSTATUS_FAIL)
	{
		return -1;
	}
	write_socket((const char*)_raw_write_buffer.read_ptr(), _raw_write_buffer.readable_size());
	_raw_write_buffer.reset();
}

int SslConnection::async_write(WriteReq* req)
{
	++_write_msg_count;
	if (!_is_close && _error == 0)
	{
		uv_write((uv_write_t*)req, (uv_stream_t*)_handle, &req->buf, 1, SslConnection::write_cb);
	}
	return 0;
}

int SslConnection::writeInLoop(const char* data, int len)
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
		_loop_ptr->runInLoop(std::bind(&SslConnection::async_write, this, req));
	}
	return 0;
}

void SslConnection::write_cb(uv_write_t* preq, int status)
{
	//std::cout << "write status: " << status << std::endl;
	WriteReq* ireq = (WriteReq*)preq;
	free(ireq->buf.base);
	SslConnection* conn = (SslConnection*)ireq->req.data;
	if (conn)
	{
		--conn->_write_msg_count;
		if (conn->_del_after_write && conn->_write_msg_count < 1)
		{
			conn->close();
		}
	}
	delete ireq;
}

void SslConnection::del_after_write()
{
	_del_after_write = true;
}

void SslConnection::on_close()
{
	_gentor.recyle(id());
	if (_ccb)
	{
		_ccb(shared_from_this());
	}
	//_gentor.recyle(id());
	//_connid = -1;//id设置为无效
}

bool SslConnection::del_handle() const
{
	return _handle_del;
}

void SslConnection::close()
{
	std::cout << "close TcpConnection: id : " << id() << std::endl;
	if (_is_close)
	{
		return;
	}
	
	if (_loop_ptr->isRunInLoopThread())
	{
		if (_is_close)
		{
			return;
		}
		_is_close = true;
		if (uv_is_active((uv_handle_t*)_handle))
		{
			uv_read_stop((uv_stream_t*)_handle);
		}
		if (uv_is_closing((uv_handle_t*)_handle) == 0)
		{
			uv_close((uv_handle_t*)_handle, [](uv_handle_t* handle) {
				SslConnection* conn = (SslConnection*)handle->data;
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
		_loop_ptr->runInLoop(std::bind(&SslConnection::close, this));
	}
}

NS_UVCORE_E