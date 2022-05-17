﻿#include "core/tcp_server.h"
#include "core/tcp_connection.h"
#include "core/event_loop.h"
#include <iostream>
#include "core/timer_event_loop.h"
#include "core/timer.h"
#include "core/udp.h"

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "Iphlpapi.lib")
#pragma comment (lib, "Psapi.lib")
#pragma comment (lib, "Userenv.lib")

void udp_recv_cb(uvcore::Udp*, uvcore::IpAddress&)
{

}

void udp_send_th(uvcore::Udp* udp)
{
	uvcore::IpAddress ip(6565);
	ip.setIp(std::string("127.0.0.1"));
	std::string buf("hello");
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(3));
		udp->send(buf.c_str(), buf.size(), ip);
	}
}


int main5()
{
	//带有定时器的event_loop，这样线程就不会退出
	std::shared_ptr<uvcore::TimerEventLoop> loop = std::make_shared<uvcore::TimerEventLoop>(3000, nullptr);

	uvcore::Udp* udp = new uvcore::Udp(loop.get());
	//uvcore::IpAddress ip(6565);
	//udp->bindAndRecv(ip, std::bind(udp_recv_cb, std::placeholders::_1, std::placeholders::_2));
	std::thread th(udp_send_th, udp);

	loop->start_timer();
	loop->run();

	return 0;
}