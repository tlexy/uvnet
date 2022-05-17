#ifndef UVCORE_UDP_SERVER_UUUUU_H
#define UVCORE_UDP_SERVER_UUUUU_H

#include "../utils/ns_helper.h"
#include <memory>
#include "udp.h"

NS_UVCORE_B

class EventLoop;
class Udp;

class UdpServer
{
public:
	UdpServer(std::shared_ptr<EventLoop> loop);

	//using UdpReceiveCallback = std::function<void(Udp*, const struct sockaddr*)>;
	//loop循环已经开始调用这个方法，可以在任意线程中调用
	Udp* addBind(IpAddress&, Udp::UdpReceiveCallback2 cb);
	//如果loop循环还没有开始，调用这个监听
	Udp* addPreBind(IpAddress&, Udp::UdpReceiveCallback2 cb);
	//Udp* addBind(IpAddress&, Udp::UdpReceiveCallback cb);

private:
	void do_bind(Udp*, IpAddress&, Udp::UdpReceiveCallback2 cb);

private:
	std::shared_ptr<EventLoop> _loop;
};

NS_UVCORE_E

#endif