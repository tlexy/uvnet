#ifndef UVCORE_SSL_SERVER_H
#define UVCORE_SSL_SERVER_H

#include <utils/ns_helper.h>
#include <chrono>
#include <thread>
#include <string>
#include <memory>
#include <uv.h>
#include <map>
#include <functional>

NS_UVCORE_B

class SslConnection;
class EventLoop;

typedef std::function<void(std::shared_ptr<SslConnection>)> SslConnectionCallBack;

class SslServer
{
public:
	SslServer() = delete;
	SslServer(std::shared_ptr<EventLoop> loop, const std::string& ip, int port);

	int start();
	void stop();
	uv_loop_t* loop();

	void on_newconnection(SslConnectionCallBack);

	void accept(uv_stream_t*);

private:
	std::shared_ptr<SslConnection> create_connection(uv_tcp_t*);

protected:
	//uv_loop_t* _loop{ NULL };
	std::shared_ptr<EventLoop> _loop_ptr;
	struct sockaddr_in _serv_addr;
	uv_tcp_t _serv_handle;
	//uv_async_t _stop_handle;
	int _port;
	std::string _ip;

	std::map<int64_t, std::shared_ptr<SslConnection>> _session_map;

	SslConnectionCallBack _conn_cb = nullptr;
};

NS_UVCORE_E

#endif