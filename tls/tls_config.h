#ifndef TLS_CONFIG_H
#define TLS_CONFIG_H

#include <openssl/ssl.h>
#include <openssl/err.h>

class TlsConfig
{
public:
	static bool init_server(const char* crt_file, const char* key_file, const char* ca_file = 0);
	static SSL_CTX* get_ssl_ctx();

	static SSL_CTX* new_ctx(const char* crt_file, const char* key_file, const char* ca_file = 0);

private:
	static void _init();
	static SSL_CTX* _new_ssl_ctx(const char* crt_file, const char* key_file, const char* ca_file = 0);
	static void _set_verify(SSL_CTX*, const char* ca_file);

private:
	static SSL_CTX* _ssl_ctx;
};

#endif