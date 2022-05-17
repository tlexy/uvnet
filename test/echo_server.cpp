#include "core/tcp_server.h"
#include "core/tcp_connection.h"
#include "core/event_loop.h"
#include <iostream>
#include <signal.h>
#include "server/general_server.h"
#include "utils/circle_buffer.h"

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "Iphlpapi.lib")
#pragma comment (lib, "Psapi.lib")
#pragma comment (lib, "Userenv.lib")

class EchoServer;

static std::shared_ptr<EchoServer> serptr;

class EchoServer : public uvcore::GeneralServer
{
protected:
	virtual void on_newconnection(std::shared_ptr<uvcore::TcpConnection> conn)
	{
		std::cout << "new connection here, id : " << conn->id() << std::endl;
	}

	virtual void on_message(std::shared_ptr<uvcore::TcpConnection>)
	{}

	virtual void on_connection_close(std::shared_ptr<uvcore::TcpConnection>)
	{}

	virtual void timer_event(uvcore::Timer*)
	{
		std::cout << "Echo server timer" << std::endl;
	}
};

void kill_signal(int signal)
{
	serptr->stop_io_server();
	std::cout << "kill process signal ..." << std::endl;
}

int main()
{
	signal(SIGINT, kill_signal);
	signal(SIGTERM, kill_signal);
#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);
#endif

#if defined(_WIN32) || defined(_WIN64)
	signal(SIGBREAK, kill_signal);
#endif
	//unsigned char buf[1320] = {0};
	//CircleBuffer cbuf(16 * 1024);
	//while (true)
	//{
	//	cbuf.write(buf, sizeof(buf));
	//	std::this_thread::sleep_for(std::chrono::seconds(1));
	//	int len = cbuf.readable_size();
	//	std::cout << "readable size: " << len << std::endl;
	//	cbuf.has_read(len);
	//}

	
	//EchoServer server;
	serptr = std::make_shared<EchoServer>();
	serptr->start_timer(5000);
	serptr->async_io_start("0.0.0.0", 3307);
	while (true)
	{
		std::string str;
		std::cin >> str;
		std::cout << "input: " << str << std::endl;
	}
	

	/*
	std::shared_ptr<uvcore::EventLoop> loop = std::make_shared<uvcore::EventLoop>();
	serptr = std::make_shared<uvcore::TcpServer>(loop, "0.0.0.0", 3307);

	serptr->on_newconnection([](std::shared_ptr<uvcore::TcpConnection> ptr) {
		std::cout << "new connection: id = " << ptr->id() << std::endl;
		ptr->set_receive_cb([](std::shared_ptr<uvcore::TcpConnection> ptr) {
			std::string str((char*)ptr->get_inner_buffer()->read_ptr(), ptr->get_inner_buffer()->readable_size());
			std::cout << str.c_str() << std::endl;
			ptr->get_inner_buffer()->has_read(str.size());
			ptr->write(str.c_str(), str.size());
		});
	});


	serptr->start();
	*/

	return 0;
}