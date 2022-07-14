#ifndef UVCORE_SSL_CHANNEL_H
#define UVCORE_SSL_CHANNEL_H

#include <utils/ns_helper.h>
#include <utils/circle_buffer.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>

NS_UVCORE_B

enum SSLStatus {
	SSLSTATUS_OK, SSLSTATUS_WANT_IO, SSLSTATUS_FAIL
};

class SslChannel
{
public:
	SslChannel(bool is_server = true);
	virtual ~SslChannel();
	SSLStatus write_to_ssl(const char* data, int len);
	SSLStatus do_ssl_handshake();
	SSLStatus get_ssl_status(int n);

	bool is_handshake_finished();
	//写入未解密的数据
	int write_to_bio(CircleBuffer*);
	//读出已经解密的数据
	int read_from_bio(CircleBuffer*);

	virtual void write_socket(const char* data, int len) = 0;

protected:
	CircleBuffer _raw_write_buffer;
	bool _is_server = true;
	SSL* _ssl = nullptr;
	BIO* _read_bio = nullptr;
	BIO* _write_bio = nullptr;
};

NS_UVCORE_E

#endif