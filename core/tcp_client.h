#ifndef UVCORE_TCP_CLIENT_H
#define UVCORE_TCP_CLIENT_H

#include "../utils/ns_helper.h"
#include <memory>
#include <uv.h>
#include <functional>
#include <stdint.h>
#include "ip_address.h"

NS_UVCORE_B

class TcpConnection;
class EventLoop;
class Timer;

typedef std::function<void(int, std::shared_ptr<TcpConnection>)> HandshadeCompleteCallBack;

class TcpClient
{
public:
	TcpClient(std::shared_ptr<uvcore::EventLoop> loop, const uvcore::IpAddress&);

	virtual ~TcpClient();

	void close();
	void close_connection();
	void set_connect_timeout(int milli); 

	//loop循环后才能调用async_connect
	void async_connect(HandshadeCompleteCallBack);
	//loop循环未开始时调用ptr_connect
	void pre_connect(HandshadeCompleteCallBack);

	std::shared_ptr<TcpConnection> get_connection();

	std::shared_ptr<uvcore::EventLoop> get_loop();

	uvcore::IpAddress addr();

	virtual void on_message(std::shared_ptr<uvcore::TcpConnection>) = 0;
	//调用_conn_ptr->close()后会调用这个函数
	virtual void on_connection_close(std::shared_ptr<uvcore::TcpConnection>) = 0;

protected:
	void on_message_(std::shared_ptr<uvcore::TcpConnection>);
	void on_connection_close_(std::shared_ptr<uvcore::TcpConnection>);

private:
	int init();
	void do_connect();
	void do_close();
	void do_close_connection();
	void on_connect_timer(uvcore::Timer*);

protected:
	std::shared_ptr<EventLoop> _loop_ptr;
	uv_tcp_t* _client_handle{NULL};
	IpAddress _client_addr;
	uv_connect_t _client_connect;
	HandshadeCompleteCallBack _cb;
	std::shared_ptr<TcpConnection> _conn_ptr;
	std::shared_ptr<Timer> _connect_timer{nullptr};
	int _connect_timeout{ -1 };
	bool _is_on_connecting{ false };

private:
	void onConnect(int status);
};

NS_UVCORE_E

#endif