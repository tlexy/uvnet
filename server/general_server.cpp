#include "general_server.h"
#include "../core/tcp_server.h"
#include "../core/tcp_connection.h"
#include "../core/event_loop.h"
#include "../core/timer.h"
#include <iostream>

NS_UVCORE_B

GeneralServer::GeneralServer()
	:_io_thread(std::shared_ptr<std::thread>()),
	_timer(std::shared_ptr<Timer>())
{
	_loop = std::make_shared<uvcore::EventLoop>();
}

void GeneralServer::async_io_start(const std::string& ip, int port)
{
	if (_io_thread)
	{
		return;
	}
	/*_loop = std::make_shared<uvcore::EventLoop>();*/
	_server = std::make_shared<uvcore::TcpServer>(_loop, ip, port);

	_server->on_newconnection([this](std::shared_ptr<uvcore::TcpConnection> ptr) {
		ptr->set_receive_cb(std::bind(&GeneralServer::on_message_, this, std::placeholders::_1));
		ptr->set_close_cb(std::bind(&GeneralServer::on_connection_close_, this, std::placeholders::_1));
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

void GeneralServer::stop_io_server()
{
	if (_io_thread)
	{
		_server->stop();
		_io_thread->join();
		_io_thread = std::shared_ptr<std::thread>();
	}
}

void GeneralServer::timer_event(uvcore::Timer*)
{
	//std::cout << "timer event..." << std::endl;
}

void GeneralServer::start_timer(int64_t millsec)
{
	if (_timer)
	{
		return;
	}
	_timer = std::make_shared<Timer>(_loop.get(), millsec, millsec, std::bind(&GeneralServer::timer_event_, this, std::placeholders::_1));
	_timer->pre_start();
}

std::shared_ptr<uvcore::EventLoop> GeneralServer::get_loop()
{
	return _loop;
}

void GeneralServer::on_message_(std::shared_ptr<uvcore::TcpConnection> ptr)
{
	on_message(ptr);
}

void GeneralServer::on_connection_close_(std::shared_ptr<uvcore::TcpConnection> ptr)
{
	on_connection_close(ptr);
}

void GeneralServer::timer_event_(Timer* timer)
{
	timer_event(timer);
}

//void GeneralServer::on_newconnection(std::shared_ptr<uvcore::TcpConnection>)
//{}
//
//void GeneralServer::on_message(std::shared_ptr<uvcore::TcpConnection>)
//{}
//
//void GeneralServer::on_connection_close(std::shared_ptr<uvcore::TcpConnection>)
//{}

NS_UVCORE_E