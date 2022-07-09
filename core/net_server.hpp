#ifndef UVCORE_NET_SERVER_H
#define UVCORE_NET_SERVER_H

#include <utils/ns_helper.h>
#include <chrono>
#include <thread>
#include <string>
#include <memory>
#include <uv.h>
#include <map>
#include <functional>
#include <iostream>

NS_UVCORE_B

class TcpConnection;
class EventLoop;

template <typename C>
class NetServer
{
public:
	//using ConnectionPtr = std::shared_ptr<C>;
	using ConnectionCallBack = std::function<void(std::shared_ptr<C>)>;

	NetServer() = delete;
	NetServer(std::shared_ptr<EventLoop> loop, const std::string& ip, int port);

	int start();
	void stop();
	uv_loop_t* loop()
	{
		return _loop_ptr->uv_loop();
	}

	void on_newconnection(ConnectionCallBack cb)
	{
		_conn_cb = cb;
	}

	void accept(uv_stream_t*);

private:
	static void on_new_connection(uv_stream_t* server, int status);
	static void alloc_buffer(uv_handle_t* handle,
		size_t suggested_size,
		uv_buf_t* buf);
	static void after_read(uv_stream_t* stream,
		ssize_t nread,
		const uv_buf_t* buf);

private:
	std::shared_ptr<C> create_connection(uv_tcp_t*);

protected:
	std::shared_ptr<EventLoop> _loop_ptr;
	struct sockaddr_in _serv_addr;
	uv_tcp_t _serv_handle;
	int _port;
	std::string _ip;

	ConnectionCallBack _conn_cb = nullptr;
};

template<typename C>
void NetServer<C>::on_new_connection(uv_stream_t* server, int status)
{
	if (status < 0)
	{
		fprintf(stderr, "New connection error %s\n", uv_strerror(status));
		return;
	}
	NetServer<C>* svr = (NetServer<C>*)server->data;
	svr->accept(server);
}

template<typename C>
void NetServer<C>::alloc_buffer(uv_handle_t* handle,
	size_t suggested_size,
	uv_buf_t* buf)
{
	C* connection = (C*)handle->data;
	buf->base = (char*)connection->get_buffer();
	buf->len = connection->get_buffer_length();
}

template<typename C>
void NetServer<C>::after_read(uv_stream_t* stream,
	ssize_t nread,
	const uv_buf_t* buf)
{
	C* connection = (C*)stream->data;
	if (nread > 0)
	{
		connection->has_written(nread);
		connection->on_receive_data(nread);
	}
	else if (nread < 0)
	{
		connection->set_error(1);
		connection->on_receive_data(0);
	}
}

template <typename C>
NetServer<C>::NetServer(std::shared_ptr<EventLoop> loop, const std::string& ip, int port)
	:_ip(ip),
	_port(port),
	_loop_ptr(loop)
{
	uv_tcp_init(_loop_ptr->uv_loop(), &_serv_handle);
	uv_ip4_addr(ip.c_str(), port, &_serv_addr);
	int ret = uv_tcp_bind(&_serv_handle, (const struct sockaddr*)&_serv_addr, 0);
	if (ret != 0)
	{
		std::cerr << "bind error" << std::endl;
		return;
	}
}

template<typename C>
int NetServer<C>::start()
{
	_serv_handle.data = this;
	int ret = uv_listen((uv_stream_t*)&_serv_handle, SOMAXCONN, NetServer<C>::on_new_connection);
	if (ret != 0)
	{
		return ret;
	}
	std::cerr << "thread work, ret=" << ret << " listening at " << _port << std::endl;

	if (_loop_ptr)
	{
		_loop_ptr->run();
	}
	return 0;
}

template<typename C>
void NetServer<C>::stop()
{
	if (_loop_ptr)
	{
		_loop_ptr->stop();
	}
}

template<typename C>
void NetServer<C>::accept(uv_stream_t* server)
{
	uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	if (client == NULL)
	{
		std::cerr << "on new connection, malloc client is NULL" << std::endl;
		return;
	}
	int r = uv_tcp_init(_loop_ptr->uv_loop(), client);
	//ASSERT(r == 0 && client->type == UV_TCP);
	r = uv_accept(server, (uv_stream_t*)client);
	//ASSERT(r == 0);

	auto ptr = create_connection(client);
	client->data = ptr.get();

	r = uv_read_start((uv_stream_t*)client, NetServer<C>::alloc_buffer, NetServer<C>::after_read);
	//ASSERT(r == 0);

	if (_conn_cb)
	{
		_conn_cb(ptr);
	}
}

template<typename C>
std::shared_ptr<C> NetServer<C>::create_connection(uv_tcp_t* handle)
{
	if (!handle)
	{
		return NULL;
	}
	std::shared_ptr<C> ptr = std::make_shared<C>(_loop_ptr, handle);
	return ptr;
}

NS_UVCORE_E

#endif