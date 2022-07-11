#ifndef UVCORE_SSL_CONNECTION_H
#define UVCORE_SSL_CONNECTION_H

#include <utils/ns_helper.h>
#include <uv.h>
#include <utils/circle_buffer.h>
#include <functional>
#include <memory>
#include <utils/ngenerator.hpp>
#include <core/tcp_connection.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>

NS_UVCORE_B


class SslConnection : public TcpConnection
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

	void set_new_ssl_cb(SslCallBack);

private:
	//int write_socket(const char* data, int len);
	SSLStatus write_to_ssl(const char* data, int len);
	SSLStatus do_ssl_handshake();
	SSLStatus get_ssl_status(int n);

private:
	// decode from receivce buffer
	CircleBuffer _dec_buffer;
	CircleBuffer _raw_write_buffer;
	SslCallBack _ncb = nullptr;

	bool _is_server = true;
	SSL* _ssl = nullptr;
	BIO* _read_bio = nullptr;
	BIO* _write_bio = nullptr;

};

NS_UVCORE_E

#endif