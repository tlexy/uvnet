#ifndef UVCORE_SSL_CONNECTION_H
#define UVCORE_SSL_CONNECTION_H

#include <utils/ns_helper.h>
#include <uv.h>
#include <utils/circle_buffer.h>
#include <functional>
#include <memory>
#include <utils/ngenerator.hpp>
#include <core/tcp_connection.h>
#include <tls/ssl_channel.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>

NS_UVCORE_B


class SslConnection : public TcpConnection, public SslChannel
{
public:
	using SslCallBack = std::function<void(std::shared_ptr<SslConnection>)>;
	enum SSLStatus {
		SSLSTATUS_OK, SSLSTATUS_WANT_IO, SSLSTATUS_FAIL
	};
	SslConnection(std::shared_ptr<EventLoop> loop, uv_tcp_t* handle, bool is_server = true, bool del = true);
	~SslConnection();

	CircleBuffer* get_dec_buffer();

	virtual void on_receive_data(size_t len);
	virtual int write(const char* data, int len);
	virtual int writeInLoop(const char* data, int len);

	virtual void write_socket(const char* data, int len);

	void set_new_ssl_cb(SslCallBack);

private:
	// decode from receivce buffer
	CircleBuffer _dec_buffer;
	SslCallBack _ncb = nullptr;

};

NS_UVCORE_E

#endif