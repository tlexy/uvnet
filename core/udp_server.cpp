#include "udp_server.h"
#include "event_loop.h"
#include <iostream>

NS_UVCORE_B

UdpServer::UdpServer(std::shared_ptr<EventLoop> loop)
	:_loop(loop)
{}

Udp* UdpServer::addBind(IpAddress& ip, Udp::UdpReceiveCallback2 cb)
{
	Udp* udp = new Udp();
	if (!udp)
	{
		if (cb)
		{
			cb(nullptr, nullptr);
		}
		return nullptr;
	}
	int ret = 0;
	if (_loop->isRunInLoopThread())
	{
		ret = do_bind(udp, ip, cb);
	}
	else
	{
		_loop->runInLoop(std::bind(&UdpServer::do_bind, this, udp, ip, cb));
	}
	/*if (ret == 0)
	{
		return udp;
	}
	return nullptr;*/
	return udp;
}

Udp* UdpServer::addPreBind(IpAddress& ip, Udp::UdpReceiveCallback2 cb)
{
	Udp* udp = new Udp();
	if (!udp)
	{
		return nullptr;
	}
	int ret = do_bind(udp, ip, cb);
	if (ret != 0)
	{
		delete udp;
		return nullptr;
	}
	return udp;
}

int UdpServer::do_bind(Udp* udp, IpAddress& ip, Udp::UdpReceiveCallback2 cb)
{
	udp->setLoop(_loop.get());
	int ret = udp->bindAndRecv2(ip, cb);
	//绑定失败时，调用回调函数，地址传为空
	if (ret != 0 && cb)
	{
		const char* errstr = uv_strerror(ret);
		std::cerr << "do bind error, return core:" << ret << ", err msg: " << errstr << ",file: " << __FUNCTION__ << ",LINE: " << __LINE__ << std::endl;
		cb(udp, nullptr);
	}
	return ret;
}

//Udp* UdpServer::addBind(IpAddress&, Udp::UdpReceiveCallback cb)
//{
//	return NULL;
//}


NS_UVCORE_E