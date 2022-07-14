#ifndef UV_CORE_WEBSOCKET_GENERAL_SERVER_H
#define UV_CORE_WEBSOCKET_GENERAL_SERVER_H

#include <utils/ns_helper.h>
#include <memory>
#include <string>
#include <thread>
#include <core/net_server.hpp>

NS_UVCORE_B

class TcpConnection;
class WsConnection;
class EventLoop;
class Timer;

class WsGeneralServer
{
public:
	using Server = uvcore::NetServer<uvcore::WsConnection>;
	WsGeneralServer();

	void async_io_start(const std::string& ip, int port);
	void stop_io_server();
	//需要在loop启动前调用
	void start_timer(int64_t millsec);

	std::shared_ptr<uvcore::EventLoop> get_loop();

protected:
	void on_message_(std::shared_ptr<uvcore::TcpConnection>);
	void on_connection_close_(std::shared_ptr<uvcore::TcpConnection>);
	void on_websocket_close_(std::shared_ptr<uvcore::WsConnection>, const std::string&);
	void on_websocket_ping_(std::shared_ptr<uvcore::WsConnection>, const std::string&);
	void on_handshake_complete_(std::shared_ptr<uvcore::WsConnection>);
	void timer_event_(Timer*);
	
	/// <summary>
	/// 
	/// </summary>
	/// <param name=""></param>
	virtual void on_newconnection(std::shared_ptr<uvcore::WsConnection>) = 0;
	virtual void on_message(std::shared_ptr<uvcore::WsConnection>) = 0;
	virtual void on_connection_close(std::shared_ptr<uvcore::WsConnection>) = 0;
	virtual void on_websocket_close(std::shared_ptr<uvcore::WsConnection>, const std::string&) = 0;
	virtual void on_websocket_ping(std::shared_ptr<uvcore::WsConnection>, const std::string&) = 0;
	virtual void on_handshake_complete(std::shared_ptr<uvcore::WsConnection>) = 0;
	virtual void timer_event(uvcore::Timer*);

protected:
	//std::shared_ptr<uvcore::TcpServer> _server;
	std::shared_ptr<Server> _server;
	std::shared_ptr<uvcore::EventLoop> _loop;
	std::shared_ptr<std::thread> _io_thread;

	std::shared_ptr<Timer> _timer{nullptr};
};

NS_UVCORE_E

#endif
