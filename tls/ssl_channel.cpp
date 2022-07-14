#include "ssl_channel.h"
#include <tls/tls_config.h>
#include <utils/global.h>
#include <iostream>

NS_UVCORE_B

SslChannel::SslChannel(bool is_server)
	:_is_server(is_server),
	_raw_write_buffer(Global::get_instance()->socket_buffer())
{
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

SslChannel::~SslChannel()
{
	//BIO_free(_read_bio);
	//BIO_free(_write_bio);
	SSL_free(_ssl);
}

bool SslChannel::is_handshake_finished()
{
	return SSL_is_init_finished(_ssl);
}

int SslChannel::write_to_bio(CircleBuffer* buffer)
{
	int n = BIO_write(_read_bio, buffer->read_ptr(), buffer->readable_size());
	if (n > 0)
	{
		buffer->has_read(n);
	}
	return n;
}

int SslChannel::read_from_bio(CircleBuffer* buffer)
{
	int n = 0;
	int total = 0;
	do {
		buffer->enable_size(2048);
		n = SSL_read(_ssl, buffer->write_ptr(), buffer->writable_size());
		if (n > 0)
		{
			buffer->has_written(n);
			total += n;
		}
	} while (n > 0);
	return total;
}

SSLStatus SslChannel::write_to_ssl(const char* data, int len)
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
			else if (!BIO_should_retry(_write_bio))
			{
				return SSLSTATUS_FAIL;
			}
		} while (n > 0);
	}
	return status;
}

SSLStatus SslChannel::do_ssl_handshake()
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
				//TcpConnection::write((const char*)_raw_write_buffer.read_ptr(), _raw_write_buffer.readable_size());
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

SSLStatus SslChannel::get_ssl_status(int n)
{
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

NS_UVCORE_E