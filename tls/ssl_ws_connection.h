#ifndef UVCORE_SSL_WEBSOCKET_CONNECTION_H
#define UVCORE_SSL_WEBSOCKET_CONNECTION_H

#include <utils/ns_helper.h>
#include <uv.h>
#include <utils/circle_buffer.h>
#include <functional>
#include <memory>
#include <utils/ngenerator.hpp>
#include <core/tcp_connection.h>
#include <tls/ssl_channel.h>
#include <string>

namespace httpparser
{
	class Request;
}

NS_UVCORE_B

class SslWsConnection : public TcpConnection, public SslChannel
{
public:
	//using DataCallBack = std::function<void(std::shared_ptr<WsConnection>)>;
	//using CloseCallBack = std::function<void(std::shared_ptr<WsConnection>)>;
	using HandshakeCallBack = std::function<void(std::shared_ptr<SslWsConnection>)>;
	using SslHandshakeCallBack = std::function<void(std::shared_ptr<SslWsConnection>)>;
	using HandleWsCloseCallBack = std::function<void(std::shared_ptr<SslWsConnection>, const std::string&)>;
	using HandlePingCallBack = std::function<void(std::shared_ptr<SslWsConnection>, const std::string&)>;

	enum ErrCode
	{
		HandshakeError = 2,
		EndError
	};
	SslWsConnection(std::shared_ptr<EventLoop> loop, uv_tcp_t* handle, bool del = true);
	~SslWsConnection();

	virtual void write_socket(const char* data, int len);

	virtual void on_receive_data(size_t len);

	virtual int write(const char* data, int len, bool is_ws = true);
	//可以在任意线程中调用
	virtual int writeInLoop(const char* data, int len);

	CircleBuffer* get_dec_buffer();
	bool is_handshake();
	void set_handshake_cb(HandshakeCallBack cb);
	/*void set_receive_cb(DataCallBack cb);
	void set_close_cb(CloseCallBack cb);*/
	void set_ws_close_cb(HandleWsCloseCallBack cb);
	void set_ws_ping_cb(HandlePingCallBack cb);

	void set_new_ssl_cb(SslHandshakeCallBack);

private:
	void do_ws_handshake();
	void handle_ws_data_frame();
	void err_close(int error);

	CircleBuffer* get_ssl_dec_buffer();

	int get_version(std::shared_ptr<httpparser::Request>);
	std::string get_header_string(std::shared_ptr<httpparser::Request>, const std::string& key);

private:
	bool _is_handshake = false;
	SslHandshakeCallBack _ncb = nullptr;
	HandshakeCallBack _hcb = nullptr;
	HandleWsCloseCallBack _wsccb = nullptr;
	HandlePingCallBack _wspcb = nullptr;
	CircleBuffer _dec_buffer;
	CircleBuffer _ssl_dec_buffer;
};

NS_UVCORE_E

#endif