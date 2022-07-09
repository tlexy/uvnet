#ifndef UVCORE_TCP_SERVER_H
#define UVCORE_TCP_SERVER_H

#include "../utils/ns_helper.h"
#include <chrono>
#include <thread>
#include <string>
#include <memory>
#include <uv.h>
#include <map>
#include <functional>

NS_UVCORE_B

class TcpConnection;
class EventLoop;

typedef std::function<void(std::shared_ptr<TcpConnection>)> ConnectionCallBack;

class TcpServer
{
public:
	TcpServer() = delete;
	TcpServer(std::shared_ptr<EventLoop> loop, const std::string& ip, int port);

	int start();
	void stop();
	uv_loop_t* loop();

	void on_newconnection(ConnectionCallBack);

	void accept(uv_stream_t*);

	std::shared_ptr<TcpConnection> find_connection(int64_t connid);

private:
	std::shared_ptr<TcpConnection> create_connection(uv_tcp_t*);

protected:
	//uv_loop_t* _loop{ NULL };
	std::shared_ptr<EventLoop> _loop_ptr;
	struct sockaddr_in _serv_addr;
	uv_tcp_t _serv_handle;
	//uv_async_t _stop_handle;
	int _port;
	std::string _ip;

	std::map<int64_t, std::shared_ptr<TcpConnection>> _session_map;

	ConnectionCallBack _conn_cb;
};

NS_UVCORE_E

#endif