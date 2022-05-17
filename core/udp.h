#ifndef UVCORE_UDP_H
#define UVCORE_UDP_H

#include "../utils/ns_helper.h"
#include <uv.h>
#include "ip_address.h"
#include "../utils/circle_buffer.h"
#include <functional>
#include "../utils/ngenerator.hpp"

//NS_UVCORE_B
//class Udp;
//NS_UVCORE_E
//
//using UdpReceiveCallback2 = std::function<void(uvcore::Udp*, const struct sockaddr*)>;
//using UdpCloseCallback = std::function<void(int64_t)>;

NS_UVCORE_B

class EventLoop;

typedef struct udp_send_s
{
	uv_udp_send_t req;
	uv_buf_t buf;
} udp_send_t;

class Udp : public SequenceBuffer
{
public:
	using UdpReceiveCallback2 = std::function<void(uvcore::Udp*, const struct sockaddr*)>;
	using UdpCloseCallback = std::function<void(int64_t)>;
	//using UdpReceiveCallback = std::function<void(Udp*, IpAddress&)>;
	Udp(EventLoop* loop);
	Udp();

	virtual ~Udp();

	int64_t id() const;

	void close(UdpCloseCallback);
	void onCompleted(int);

	void setLoop(EventLoop*);

	int get_bind_port();

	//为发送socket绑定一个地址，否则将随机分配
	void bindSend(const IpAddress&);
	//void bindAndRecv(const IpAddress&, UdpReceiveCallback cb);
	void bindAndRecv2(const IpAddress&, UdpReceiveCallback2 cb);

	void sendInLoop(const char* data, int len, IpAddress& ip);
	void send(const char* data, int len, IpAddress& ip);
	//通过绑定的ip+port发送（调用bindAndRecv）
	void sendInLoop2(const char* data, int len, IpAddress& ip);
	void send2(const char* data, int len, IpAddress& ip);

	//void onReceiveData(IpAddress&);
	void onReceiveData(const struct sockaddr* addr);

	static void send_cb(uv_udp_send_t* req, int status);

private:
	void do_send(udp_send_t*, IpAddress&);
	void do_send2(udp_send_t*, IpAddress&);
	udp_send_t* get_send_req(const char* data, int len);
	void do_close();

private:
	int64_t _id;
	EventLoop* _loop;
	uv_udp_t* _send_udp{NULL};
	uv_udp_t* _recv_udp{NULL};
	IpAddress _bind_addr;
	//UdpReceiveCallback _recv_cb{};
	UdpReceiveCallback2 _recv_cb2{};
	UdpCloseCallback _close_cb;

	static NGenerator<int64_t> _gentor;
};

NS_UVCORE_E

#endif