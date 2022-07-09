#ifndef UVCORE_SSL_CONNECTION_H
#define UVCORE_SSL_CONNECTION_H

#include <utils/ns_helper.h>
#include <uv.h>
#include <utils/circle_buffer.h>
#include <functional>
#include <memory>
#include <utils/ngenerator.hpp>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>

NS_UVCORE_B

struct WriteReq
{
	uv_write_t req;
	uv_buf_t buf;
};

class Packer;
class SslConnection;
class EventLoop;

typedef std::function<void(std::shared_ptr<SslConnection>)> SslCallBack;

class SslConnection : public std::enable_shared_from_this<SslConnection>
{
public:
	enum SSLStatus {
		SSLSTATUS_OK, SSLSTATUS_WANT_IO, SSLSTATUS_FAIL
	};
	SslConnection(std::shared_ptr<EventLoop> loop, uv_tcp_t* handle, bool is_server = true, bool del = true);
	~SslConnection();

	int64_t id() { return _connid; }

	void calc_ip();

	uint32_t get_remote_ip();

	char* get_buffer();
	uint32_t get_buffer_length();

	CircleBuffer* get_inner_buffer();

	CircleBuffer* get_dec_buffer();

	bool del_handle() const;

	void set_error(int);
	int error();
	//与上层业务相关的state
	void set_state(int);
	int state();

	void set_create_time(int64_t);
	int64_t create_time();

	std::shared_ptr<EventLoop> loop();

	void has_written(size_t len);
	void on_receive_data(size_t len);

	void set_receive_cb(SslCallBack cb);
	void set_close_cb(SslCallBack cb);
	void set_new_ssl_cb(SslCallBack);

	void close();

	//怎么发送数据？
	//只能在loop线程中调用
	int write(const char* data, int len);
	//可以在任意线程中调用
	int writeInLoop(const char* data, int len);

	void del_after_write();

	void on_close();

	static void write_cb(uv_write_t* preq, int status);

private:
	int async_write(WriteReq*);

	int write_socket(const char* data, int len);

	SSLStatus do_ssl_handshake();
	SSLStatus get_ssl_status(int n);

private:
	uv_tcp_t* _handle{ NULL };
	std::shared_ptr<EventLoop> _loop_ptr;
	int64_t _connid;
	//receive buffer
	CircleBuffer _buffer;
	// decode from receivce buffer
	CircleBuffer _dec_buffer;

	CircleBuffer _raw_write_buffer;
	SslCallBack _cb;
	SslCallBack _ccb;
	SslCallBack _ncb = nullptr;
	bool _is_close{false};
	bool _del_after_write{false};
	static NGenerator<int64_t> _gentor;
	bool _handle_del{ true };//是否在connection关闭时free uv_tcp_t的内存
	int _error{0};
	int _state{ 0 };
	uint32_t _remote_ip{ 0 };
	int64_t _create_time;
	int _write_msg_count{ 0 };

	bool _is_server = true;
	SSL* _ssl = nullptr;
	BIO* _read_bio = nullptr;
	BIO* _write_bio = nullptr;

};

NS_UVCORE_E

#endif