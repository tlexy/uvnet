
#include "udp.h"
#include "event_loop.h"
#include <iostream>
#include <string.h>
#ifndef _WIN32
#include <jemalloc/jemalloc.h>
#endif

NS_UVCORE_B

extern "C"
{
	void on_data_receive(uv_udp_t* req, ssize_t nread,
		const uv_buf_t* buf,
		const struct sockaddr* addr, unsigned flags)
	{
		//std::cout << "addr: " << (void*)addr << std::endl;
		Udp* udp = (Udp*)req->data;
		if (nread <= 0)
		{
			// < 0 if a transmission error was detected
			//std::cout << "receive udp data error, read: " << nread << "\tflag: " << flags << std::endl;
			return;
		}
		udp->get_inner_buffer()->has_written(nread);
		/*IpAddress ip = IpAddress::fromRawSocketAddress((sockaddr*)addr, sizeof(sockaddr_in));
		std::cout << "receive data from: " << ip.toString() << "\t len: " << nread << std::endl;*/
		udp->onReceiveData(addr);
	}

	void uv_alloc(uv_handle_t* handle,
		size_t suggested_size,
		uv_buf_t* buf)
	{
		Udp* udp = (Udp*)handle->data;
		buf->base = udp->get_buffer();
		buf->len = udp->get_buffer_length();
		int a = 1;
	}
}

NGenerator<int64_t> Udp::_gentor;

Udp::Udp(EventLoop* loop)
	:SequenceBuffer(4096), 
	_send_udp(new uv_udp_t),
	_recv_udp(new uv_udp_t),
	_loop(loop),
	_id(_gentor.get_next())
{
	uv_udp_init(loop->uv_loop(), _send_udp);
	uv_udp_init(loop->uv_loop(), _recv_udp);
}

Udp::Udp()
	:SequenceBuffer(4096), 
	_send_udp(new uv_udp_t),
	_recv_udp(new uv_udp_t),
	_id(_gentor.get_next())
{

}


Udp::~Udp()
{
	if (_send_udp)
	{
		free(_send_udp);
	}
	_send_udp = NULL;

	if (_recv_udp)
	{
		free(_recv_udp);
	}
	_recv_udp = NULL;
}

int64_t Udp::id() const
{
	return _id;
}

void Udp::setLoop(EventLoop* loop)
{
	_loop = loop;
	uv_udp_init(loop->uv_loop(), _send_udp);
	uv_udp_init(loop->uv_loop(), _recv_udp);
}

int Udp::get_bind_port()
{
	return _bind_addr.getPort();
}

void Udp::bindSend(const IpAddress& ip)
{
	//struct sockaddr_in addr;
	//uv_ip4_addr(ip.toString(false).c_str(), ip.port(), &addr);
	uv_udp_bind(_send_udp, (const struct sockaddr*)ip.rawAddressPtr(), 0);
}

//void Udp::bindAndRecv(const IpAddress& ip, UdpReceiveCallback cb)
//{
//	_bind_addr = ip;
//	_recv_cb = cb;
//	uv_udp_bind(_recv_udp, (const struct sockaddr*)ip.rawAddressPtr(), UV_UDP_REUSEADDR);
//	_recv_udp->data = this;
//	uv_udp_recv_start(_recv_udp, &uv_alloc, &on_data_receive);
//}

void Udp::bindAndRecv2(const IpAddress& ip, UdpReceiveCallback2 cb)
{
	_bind_addr = ip;
	_recv_cb2 = cb;
	uv_udp_bind(_recv_udp, (const struct sockaddr*)ip.rawAddressPtr(), UV_UDP_REUSEADDR);
	_recv_udp->data = this;
	uv_udp_recv_start(_recv_udp, &uv_alloc, &on_data_receive);
}

//void Udp::onReceiveData(IpAddress& ip)
//{
//	if (_recv_cb)
//	{
//		_recv_cb(this, ip);
//	}
//}

void Udp::onReceiveData(const struct sockaddr* addr)
{
	if (_recv_cb2)
	{
		_recv_cb2(this, addr);
	}
}

udp_send_t* Udp::get_send_req(const char* data, int len)
{
	udp_send_t* send_req = (udp_send_t*)malloc(sizeof(udp_send_t));
	if (!send_req)
	{
		return NULL;
	}
	send_req->buf.base = (char*)malloc(len);
	if (!send_req->buf.base)
	{
		return NULL;
	}
	memcpy(send_req->buf.base, data, len);
	send_req->buf.len = len;
	return send_req;
}

void Udp::sendInLoop(const char* data, int len, IpAddress& ip)
{
	if (!_loop)
	{
		return;
	}
	if (_loop->isRunInLoopThread())
	{
		sendInLoop(data, len, ip);
	}
	else
	{
		udp_send_t* req = get_send_req(data, len);
		_loop->runInLoop(std::bind(&Udp::do_send, this, req, ip));
	}
	
}

void Udp::send(const char* data, int len, IpAddress& ip)
{
	udp_send_t* req = get_send_req(data, len);
	do_send(req, ip);
}

void Udp::sendInLoop2(const char* data, int len, IpAddress& ip)
{
	if (!_loop)
	{
		return;
	}
	if (_loop->isRunInLoopThread())
	{
		send2(data, len, ip);
	}
	else
	{
		udp_send_t* req = get_send_req(data, len);
		_loop->runInLoop(std::bind(&Udp::do_send2, this, req, ip));
	}
}

void Udp::send2(const char* data, int len, IpAddress& ip)
{
	udp_send_t* req = get_send_req(data, len);
	do_send2(req, ip);
}

void Udp::send_cb(uv_udp_send_t* req, int status)
{
	udp_send_t* send_req = (udp_send_t*)req;
	free(send_req->buf.base);
	free(send_req);
}

void Udp::do_send(udp_send_t* send_req, IpAddress& ip)
{
	if (ip() && send_req && send_req->buf.base)
	{
		uv_udp_send((uv_udp_send_t*)send_req, _send_udp, &send_req->buf, 1, ip.rawAddressPtr(), Udp::send_cb);
	}
	else
	{
		std::cout << "udp send error, ip maybe error: " << ip.toString().c_str() << std::endl;
		if (send_req)
		{
			if (send_req->buf.base)
			{
				free(send_req->buf.base);
			}
			free(send_req);
		}
	}
}

void Udp::do_send2(udp_send_t* send_req, IpAddress& ip)
{
	if (ip())
	{
		int ret = uv_udp_send((uv_udp_send_t*)send_req, _recv_udp, &send_req->buf, 1, ip.rawAddressPtr(), Udp::send_cb);
		if (ret != 0)
		{
			std::cout << "send udp data failed, error = " << ret << std::endl;
		}
	}
	else
	{
		std::cout << "udp send error, ip maybe error: " << ip.toString().c_str() << std::endl;
		if (send_req)
		{
			if (send_req->buf.base)
			{
				free(send_req->buf.base);
			}
			free(send_req);
		}
	}
}

void Udp::close(UdpCloseCallback cb)
{
	_send_udp->data = this;
	_recv_udp->data = this;
	_close_cb = cb;
	if (_loop->isRunInLoopThread())
	{
		do_close();
	}
	else
	{
		_loop->runInLoop(std::bind(&Udp::do_close, this));
	}
}

void Udp::onCompleted(int flag)
{
	if (flag == 1 && _send_udp)
	{
		free(_send_udp);
		_send_udp = NULL;
	}
	if (flag == 2 && _recv_udp)
	{
		free(_recv_udp);
		_recv_udp = NULL;
	}

	if (_recv_udp == NULL && _send_udp == NULL)
	{
		_gentor.recyle(_id);
		if (_close_cb)
		{
			_close_cb(_id);
		}
	}
}

void Udp::do_close()
{
	if (uv_is_active((uv_handle_t*)_send_udp))
	{
		uv_udp_recv_stop(_send_udp);
	}
	if (uv_is_closing((uv_handle_t*)_send_udp) == 0)
	{
		::uv_close((uv_handle_t*)_send_udp, [](uv_handle_t* handle)
			{
				Udp* udp = static_cast<Udp*>(handle->data);
				udp->onCompleted(1);
			});
	}
	//
	if (uv_is_active((uv_handle_t*)_recv_udp))
	{
		uv_udp_recv_stop(_recv_udp);
	}
	if (uv_is_closing((uv_handle_t*)_recv_udp) == 0)
	{
		::uv_close((uv_handle_t*)_recv_udp, [](uv_handle_t* handle)
			{
				Udp* udp = static_cast<Udp*>(handle->data);
				udp->onCompleted(2);
			});
	}
}


NS_UVCORE_E
