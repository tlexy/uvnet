#include "udp_server.h"
#include "event_loop.h"

NS_UVCORE_B

UdpServer::UdpServer(std::shared_ptr<EventLoop> loop)
	:_loop(loop)
{}

Udp* UdpServer::addBind(IpAddress& ip, Udp::UdpReceiveCallback2 cb)
{
	Udp* udp = new Udp();
	if (_loop->isRunInLoopThread())
	{
		do_bind(udp, ip, cb);
	}
	else
	{
		_loop->runInLoop(std::bind(&UdpServer::do_bind, this, udp, ip, cb));
	}
	return udp;
}

Udp* UdpServer::addPreBind(IpAddress& ip, Udp::UdpReceiveCallback2 cb)
{
	Udp* udp = new Udp();
	do_bind(udp, ip, cb);
	return udp;
}

void UdpServer::do_bind(Udp* udp, IpAddress& ip, Udp::UdpReceiveCallback2 cb)
{
	udp->setLoop(_loop.get());
	udp->bindAndRecv2(ip, cb);
}

//Udp* UdpServer::addBind(IpAddress&, Udp::UdpReceiveCallback cb)
//{
//	return NULL;
//}


NS_UVCORE_E