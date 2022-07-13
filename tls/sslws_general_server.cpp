#include <tls/sslws_general_server.h>
#include <tls/ssl_server.h>
#include <tls/ssl_ws_connection.h>
#include <core/event_loop.h>
#include <core/timer.h>
#include <iostream>

NS_UVCORE_B

SslWsGeneralServer::SslWsGeneralServer()
	:_io_thread(std::shared_ptr<std::thread>()),
	_timer(std::shared_ptr<Timer>())
{
	_loop = std::make_shared<uvcore::EventLoop>();
}

void SslWsGeneralServer::async_io_start(const std::string& ip, int port)
{
	if (_io_thread)
	{
		return;
	}
	/*_loop = std::make_shared<uvcore::EventLoop>();*/
	//_server = std::make_shared<uvcore::SslServer>(_loop, ip, port);
	_server = std::make_shared<Server>(_loop, ip, port);

	_server->on_newconnection([this](std::shared_ptr<uvcore::SslWsConnection> ptr) {
		ptr->set_receive_cb(std::bind(&SslWsGeneralServer::on_message_, this, std::placeholders::_1));
		ptr->set_close_cb(std::bind(&SslWsGeneralServer::on_connection_close_, this, std::placeholders::_1));

		ptr->set_new_ssl_cb(std::bind(&SslWsGeneralServer::on_ssl_new_, this, std::placeholders::_1));

		ptr->set_handshake_cb(std::bind(&SslWsGeneralServer::on_handshake_complete_, this, std::placeholders::_1));
		ptr->set_ws_close_cb(std::bind(&SslWsGeneralServer::on_websocket_close_, this, std::placeholders::_1, std::placeholders::_2));
		ptr->set_ws_ping_cb(std::bind(&SslWsGeneralServer::on_websocket_ping_, this, std::placeholders::_1, std::placeholders::_2));

		on_newconnection(ptr);
	});

	_io_thread = std::make_shared<std::thread>([this]() {
		int ret = _server->start();
		if (ret != 0)
		{
			std::cout << "[ERROR][ERROR][ERROR]server start maybe failed!!!!!!!!!!!!!!!!!!!" << __FILE__ << ": " << __FUNCTION__ << std::endl;
		}
	});
}

void SslWsGeneralServer::stop_io_server()
{
	if (_io_thread)
	{
		_server->stop();
		_io_thread->join();
		_io_thread = std::shared_ptr<std::thread>();
	}
}

void SslWsGeneralServer::timer_event(uvcore::Timer*)
{
	//std::cout << "timer event..." << std::endl;
}

void SslWsGeneralServer::start_timer(int64_t millsec)
{
	if (_timer)
	{
		return;
	}
	_timer = std::make_shared<Timer>(_loop.get(), millsec, millsec, std::bind(&SslWsGeneralServer::timer_event_, this, std::placeholders::_1));
	_timer->pre_start();
}

std::shared_ptr<uvcore::EventLoop> SslWsGeneralServer::get_loop()
{
	return _loop;
}

void SslWsGeneralServer::on_message_(std::shared_ptr<uvcore::TcpConnection> ptr)
{
	auto pptr = std::dynamic_pointer_cast<uvcore::SslWsConnection>(ptr);
	on_message(pptr);
}

void SslWsGeneralServer::on_ssl_new_(std::shared_ptr<uvcore::SslWsConnection> ptr)
{
	on_ssl_new(ptr);
}

void SslWsGeneralServer::on_connection_close_(std::shared_ptr<uvcore::TcpConnection> ptr)
{
	auto pptr = std::dynamic_pointer_cast<uvcore::SslWsConnection>(ptr);
	on_connection_close(pptr);
}

void SslWsGeneralServer::timer_event_(Timer* timer)
{
	timer_event(timer);
}

void SslWsGeneralServer::on_websocket_close_(std::shared_ptr<uvcore::SslWsConnection> ptr, const std::string& text)
{
	on_websocket_close(ptr, text);
}

void SslWsGeneralServer::on_websocket_ping_(std::shared_ptr<uvcore::SslWsConnection> ptr, const std::string& text)
{
	on_websocket_ping(ptr, text);
}

void SslWsGeneralServer::on_handshake_complete_(std::shared_ptr<uvcore::SslWsConnection> ptr)
{
	on_handshake_complete(ptr);
}

NS_UVCORE_E