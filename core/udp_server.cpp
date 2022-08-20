#include "udp_server.h"
#include "event_loop.h"

NS_UVCORE_B

UdpServer::UdpServer(std::shared_ptr<EventLoop> loop)
	:_loop(loop)
{}

Udp* UdpServer::addBind(IpAddress& ip, Udp::UdpReceiveCallback2 cb)
{
	Udp* udp = new Udp();
	int ret = 0;
	if (_loop->isRunInLoopThread())
	{
		ret = do_bind(udp, ip, cb);
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

int UdpServer::do_bind(Udp* udp, IpAddress& ip, Udp::UdpReceiveCallback2 cb)
{
	udp->setLoop(_loop.get());
	int ret = udp->bindAndRecv2(ip, cb);
	//绑定失败时，调用回调函数，地址传为空
	if (ret != 0 && cb)
	{
		cb(udp, nullptr);
	}
	return ret;
}

//Udp* UdpServer::addBind(IpAddress&, Udp::UdpReceiveCallback cb)
//{
//	return NULL;
//}


NS_UVCORE_E