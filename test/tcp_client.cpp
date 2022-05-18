#include "core/tcp_server.h"
#include "core/tcp_connection.h"
#include "core/tcp_client.h"
#include "core/event_loop.h"
#include <iostream>
#include "core/timer_event_loop.h"
#include "core/timer.h"
#include <memory>
#include "core/ip_address.h"

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "Iphlpapi.lib")
#pragma comment (lib, "Psapi.lib")
#pragma comment (lib, "Userenv.lib")

class ATcpClient : public uvcore::TcpClient
{
public:
	ATcpClient(std::shared_ptr<uvcore::EventLoop> loop, const uvcore::IpAddress& addr)
		:TcpClient(loop, addr)
	{}

	virtual void on_message(std::shared_ptr<uvcore::TcpConnection> ptr)
	{
		std::string recv_msg((char*)ptr->get_inner_buffer()->read_ptr(), ptr->get_inner_buffer()->readable_size());
		std::cout << "on message: " << recv_msg.c_str() << std::endl;

		ptr->get_inner_buffer()->has_read(recv_msg.size());
	}
	
	virtual void on_connection_close(std::shared_ptr<uvcore::TcpConnection> ptr)
	{}

	void on_connected(int status, std::shared_ptr<uvcore::TcpConnection> ptr)
	{
		if (status == 0)
		{
			std::string hello("hello, uvnet");
			std::cout << "connected." << std::endl;
			ptr->write(hello.c_str(), hello.size());
		}
		else
		{
			std::cout << "connect failed" << std::endl;
		}
	}
};

int main()
{
	//带有定时器的event_loop，这样线程就不会退出
	std::shared_ptr<uvcore::TimerEventLoop> loop = std::make_shared<uvcore::TimerEventLoop>(3000, nullptr);

	uvcore::IpAddress ip(3307);
	ip.setIp(std::string("127.0.0.1"));
	auto client = std::make_shared<ATcpClient>(loop, ip);
	client->pre_connect(std::bind(&ATcpClient::on_connected, client.get(), std::placeholders::_1, std::placeholders::_2));

	loop->start_timer();
	loop->run();

	return 0;
}