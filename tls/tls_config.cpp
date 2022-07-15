#include <tls/tls_config.h>
#include <iostream>
#include <mutex>

using namespace std;

SSL_CTX* TlsConfig::_ssl_ctx = nullptr;

bool TlsConfig::init_server(const char* crt_file, const char* key_file, const char* ca_file)
{
    _init();
	_ssl_ctx = _new_ssl_ctx(crt_file, key_file, ca_file);
	return _ssl_ctx != nullptr;
}

void TlsConfig::_init()
{
    static std::once_flag f;
    std::call_once(f, []() {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        ERR_load_BIO_strings();
        ERR_load_crypto_strings();
        });
}

SSL_CTX* TlsConfig::get_ssl_ctx()
{
	return _ssl_ctx;
}

SSL_CTX* TlsConfig::new_ctx(const char* crt_file, const char* key_file, const char* ca_file)
{
	return _new_ssl_ctx(crt_file, key_file, ca_file);
}

//验证证书的回调
static int SSLVerifyCB(int preverify_ok, X509_STORE_CTX* x509_ctx)
{
    if (preverify_ok == 0)
    {
        cout << "SSL cert validate failed!" << endl;
    }
    else
    {
        cout << "SSL cert validate success!" << endl;
    }
    //可以做进一步验证，比如验证证书中的域名是否正确
    return preverify_ok;
}

void TlsConfig::_set_verify(SSL_CTX* ssl_ctx, const char* ca_file)
{
    if (!ssl_ctx)return;
    if (!ca_file)
    {
        SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);
        return;
    }
    //设置验证对方证书
    SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, SSLVerifyCB);
    SSL_CTX_load_verify_locations(ssl_ctx, ca_file, 0);
}

SSL_CTX* TlsConfig::_new_ssl_ctx(const char* crt_file, const char* key_file, const char* ca_file)
{
    //创建服务器 ssl ctx上下文
    SSL_CTX* ssl_ctx = SSL_CTX_new(SSLv23_server_method());
    if (!ssl_ctx)
    {
        cerr << "SSL_CTX_new TLS_server_method failed!" << endl;
        return nullptr;
    }

    //加载证书，私钥，并验证
    int re = SSL_CTX_use_certificate_file(ssl_ctx, crt_file, SSL_FILETYPE_PEM);
    if (re <= 0)
    {
        ERR_print_errors_fp(stderr);
        return nullptr;
    }
    cout << "Load certificate success!" << endl;
    re = SSL_CTX_use_PrivateKey_file(ssl_ctx, key_file, SSL_FILETYPE_PEM);
    if (re <= 0)
    {
        ERR_print_errors_fp(stderr);
        return nullptr;
    }
    cout << "Load PrivateKey success!" << endl;

    re = SSL_CTX_check_private_key(ssl_ctx);
    if (re <= 0)
    {
        cout << "private key does not match the certificate!" << endl;
        return nullptr;
    }
    cout << "check_private_key success!" << endl;

    //对客户端证书验证
    _set_verify(ssl_ctx, ca_file);
    return ssl_ctx;
}