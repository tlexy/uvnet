#ifndef UV_CORE_GENERAL_SERVER_H
#define UV_CORE_GENERAL_SERVER_H

#include "../utils/ns_helper.h"
#include <memory>
#include <string>
#include <thread>
#include <core/net_server.hpp>

NS_UVCORE_B

//class TcpServer;
class TcpConnection;
class EventLoop;
class Timer;

class GeneralServer
{
public:
	using Server = uvcore::NetServer<uvcore::TcpConnection>;
	GeneralServer();

	void async_io_start(const std::string& ip, int port);
	void stop_io_server();
	//需要在loop启动前调用
	void start_timer(int64_t millsec);

	std::shared_ptr<uvcore::EventLoop> get_loop();

protected:
	void on_message_(std::shared_ptr<uvcore::TcpConnection>);
	void on_connection_close_(std::shared_ptr<uvcore::TcpConnection>);
	void timer_event_(Timer*);
	
	/// <summary>
	/// 
	/// </summary>
	/// <param name=""></param>
	virtual void on_newconnection(std::shared_ptr<uvcore::TcpConnection>) = 0;
	virtual void on_message(std::shared_ptr<uvcore::TcpConnection>) = 0;
	virtual void on_connection_close(std::shared_ptr<uvcore::TcpConnection>) = 0;
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
