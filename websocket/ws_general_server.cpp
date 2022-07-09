#include <websocket/ws_general_server.h>
#include <websocket/ws_connection.h>
#include <core/event_loop.h>
#include <core/timer.h>
#include <iostream>

NS_UVCORE_B

WsGeneralServer::WsGeneralServer()
	:_io_thread(std::shared_ptr<std::thread>()),
	_timer(std::shared_ptr<Timer>())
{
	_loop = std::make_shared<uvcore::EventLoop>();
}

void WsGeneralServer::async_io_start(const std::string& ip, int port)
{
	if (_io_thread)
	{
		return;
	}
	_server = std::make_shared<Server>(_loop, ip, port);

	_server->on_newconnection([this](std::shared_ptr<uvcore::WsConnection> ptr) {
		ptr->set_receive_cb(std::bind(&WsGeneralServer::on_message_, this, std::placeholders::_1));
		ptr->set_close_cb(std::bind(&WsGeneralServer::on_connection_close_, this, std::placeholders::_1));
		ptr->set_handshake_cb(std::bind(&WsGeneralServer::on_handshake_complete_, this, std::placeholders::_1));
		on_newconnection(ptr);
	});

	_io_thread = std::make_shared<std::thread>([this]() {
		int ret = _server->start();
		if (ret != 0)
		{
			std::cerr << "[ERROR][ERROR][ERROR]server start maybe failed!!!!!!!!!!!!!!!!!!!" << __FILE__ << ": " << __FUNCTION__ << std::endl;
		}
	});
}

void WsGeneralServer::stop_io_server()
{
	if (_io_thread)
	{
		_server->stop();
		_io_thread->join();
		_io_thread = std::shared_ptr<std::thread>();
	}
}

void WsGeneralServer::timer_event(uvcore::Timer*)
{
	//std::cout << "timer event..." << std::endl;
}

void WsGeneralServer::start_timer(int64_t millsec)
{
	if (_timer)
	{
		return;
	}
	_timer = std::make_shared<Timer>(_loop.get(), millsec, millsec, std::bind(&WsGeneralServer::timer_event_, this, std::placeholders::_1));
	_timer->pre_start();
}

std::shared_ptr<uvcore::EventLoop> WsGeneralServer::get_loop()
{
	return _loop;
}

void WsGeneralServer::on_message_(std::shared_ptr<uvcore::TcpConnection> ptr)
{
	auto pptr = std::dynamic_pointer_cast<uvcore::WsConnection>(ptr);
	on_message(pptr);
}

void WsGeneralServer::on_connection_close_(std::shared_ptr<uvcore::TcpConnection> ptr)
{
	auto pptr = std::dynamic_pointer_cast<uvcore::WsConnection>(ptr);
	on_connection_close(pptr);
}

void WsGeneralServer::on_handshake_complete_(std::shared_ptr<uvcore::WsConnection> ptr)
{
	on_handshake_complete(ptr);
}

void WsGeneralServer::timer_event_(Timer* timer)
{
	timer_event(timer);
}

NS_UVCORE_E