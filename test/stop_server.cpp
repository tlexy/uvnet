#include "core/tcp_server.h"
#include "core/tcp_connection.h"
#include "core/event_loop.h"
#include <iostream>
#include "core/timer_event_loop.h"
#include "core/timer.h"

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "Iphlpapi.lib")
#pragma comment (lib, "Psapi.lib")
#pragma comment (lib, "Userenv.lib")

std::shared_ptr<uvcore::TcpServer> serptr;

void thread1()
{
	std::this_thread::sleep_for(std::chrono::seconds(7));
	serptr->stop();
}

int main2()
{
	std::shared_ptr<uvcore::EventLoop> loop = std::make_shared<uvcore::EventLoop>();
	serptr = std::make_shared<uvcore::TcpServer>(loop, "0.0.0.0", 3307);

	serptr->on_newconnection([](std::shared_ptr<uvcore::TcpConnection> ptr) {
		std::cout << "new connection: id = " << ptr->id() << std::endl;
		ptr->set_receive_cb([](std::shared_ptr<uvcore::TcpConnection> ptr) {
			std::cout << "buffer size: " << ptr->get_inner_buffer()->readable_size() << std::endl;
		});
	});

	std::thread th(thread1);

	serptr->start();

	th.join();

	return 0;
}

int main3()
{
	//带有定时器的event_loop，这样线程就不会退出
	std::shared_ptr<uvcore::TimerEventLoop> loop = std::make_shared<uvcore::TimerEventLoop>(3000, nullptr);

	loop->start_timer();
	loop->run();

	return 0;
}