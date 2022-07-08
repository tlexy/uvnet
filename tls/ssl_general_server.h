#ifndef UV_CORE_SSL_GENERAL_SERVER_H
#define UV_CORE_SSL_GENERAL_SERVER_H

#include <utils/ns_helper.h>
#include <memory>
#include <string>
#include <thread>

NS_UVCORE_B

class SslServer;
class SslConnection;
class EventLoop;
class Timer;

class SslGeneralServer
{
public:
	SslGeneralServer();

	void async_io_start(const std::string& ip, int port);
	void stop_io_server();
	//需要在loop启动前调用
	void start_timer(int64_t millsec);

	std::shared_ptr<uvcore::EventLoop> get_loop();

protected:
	void on_message_(std::shared_ptr<uvcore::SslConnection>);
	void on_connection_close_(std::shared_ptr<uvcore::SslConnection>);
	void on_ssl_new_(std::shared_ptr<uvcore::SslConnection>);
	void timer_event_(Timer*);
	
	/// <summary>
	/// 
	/// </summary>
	/// <param name=""></param>
	virtual void on_newconnection(std::shared_ptr<uvcore::SslConnection>) = 0;
	virtual void on_ssl_new(std::shared_ptr<uvcore::SslConnection>) = 0;
	virtual void on_message(std::shared_ptr<uvcore::SslConnection>) = 0;
	virtual void on_connection_close(std::shared_ptr<uvcore::SslConnection>) = 0;
	virtual void timer_event(uvcore::Timer*);

protected:
	std::shared_ptr<uvcore::SslServer> _server;
	std::shared_ptr<uvcore::EventLoop> _loop;
	std::shared_ptr<std::thread> _io_thread;

	std::shared_ptr<Timer> _timer{nullptr};
};

NS_UVCORE_E

#endif
