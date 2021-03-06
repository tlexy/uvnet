#include "core/tcp_server.h"
#include "core/tcp_connection.h"
#include "core/event_loop.h"
#include <iostream>
#include <core/timer_event_loop.h>
#include <core/timer.h>
#include <core/udp.h>

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "Iphlpapi.lib")
#pragma comment (lib, "Psapi.lib")
#pragma comment (lib, "Userenv.lib")

void udp_recv_cb(uvcore::Udp* udp, const struct sockaddr*)
{
	std::string recv_msg((char*)udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size());
	std::cout << "recv udp： " << recv_msg.c_str() << std::endl;
	udp->get_inner_buffer()->reset();
}


int main()
{
	//带有定时器的event_loop，这样线程就不会退出
	std::shared_ptr<uvcore::TimerEventLoop> loop = std::make_shared<uvcore::TimerEventLoop>(3000, nullptr);

	uvcore::Udp* udp = new uvcore::Udp(loop.get());
	uvcore::IpAddress ip(6565);
	udp->bindAndRecv2(ip, std::bind(udp_recv_cb, std::placeholders::_1, std::placeholders::_2));

	loop->start_timer();
	loop->run();

	return 0;
}