#include <tls/ssl_server.h>
#include <iostream>
#include <tls/ssl_connection.h>
#include <core/event_loop.h>


#define ASSERT(expr)                                      \
 do {                                                     \
  if (!(expr)) {                                          \
    fprintf(stderr,                                       \
            "Assertion failed in %s on line %d: %s\n",    \
            __FILE__,                                     \
            __LINE__,                                     \
            #expr);                                       \
    abort() ;                                              \
  }                                                       \
 } while (0)

NS_UVCORE_B

extern "C"
{
	void on_new_ssl_connection(uv_stream_t* server, int status)
	{
		if (status < 0)
		{
			fprintf(stderr, "New connection error %s\n", uv_strerror(status));
			return;
		}
		SslServer* svr = (SslServer*)server->data;
		svr->accept(server);
	}

	void ssl_alloc_buffer(uv_handle_t* handle,
		size_t suggested_size,
		uv_buf_t* buf)
	{
		SslConnection* connection = (SslConnection*)handle->data;
		buf->base = (char*)connection->get_buffer();
		buf->len = connection->get_buffer_length();
	}

	void ssl_after_read(uv_stream_t* stream,
		ssize_t nread,
		const uv_buf_t* buf)
	{
		SslConnection* connection = (SslConnection*)stream->data;
		if (nread > 0)
		{
			connection->has_written(nread);
			connection->on_receive_data(nread);
		}
		else if (nread < 0)
		{
			//std::cout << "connection close, id = " << connection->id() << std::endl;
			//connection->close();
			connection->set_error(1);
			connection->on_receive_data(0);
		}
		else
		{
			//Note nread might be 0, which does not indicate an error or EOF. This is equivalent to EAGAIN or EWOULDBLOCK under read(2).
		}
	}
}

SslServer::SslServer(std::shared_ptr<EventLoop> loop, const std::string& ip, int port)
	:_ip(ip),
	_port(port),
	_loop_ptr(loop)
{
	//_loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
	//uv_loop_init(_loop);

	uv_tcp_init(_loop_ptr->uv_loop(), &_serv_handle);
	uv_ip4_addr(ip.c_str(), port, &_serv_addr);
	int ret = uv_tcp_bind(&_serv_handle, (const struct sockaddr*)&_serv_addr, 0);
	if (ret != 0)
	{
		std::cout << "bind error" << std::endl;
		return;
	}

	/*uv_async_init(_loop, &_stop_handle, [](uv_async_t* handle) {
		std::cout << "over..." << std::endl;
		free(handle);
		});*/
}

int SslServer::start()
{
	_serv_handle.data = this;
	int ret = uv_listen((uv_stream_t*)&_serv_handle, SOMAXCONN, on_new_ssl_connection);
	if (ret != 0)
	{
		return ret;
	}
	std::cout << "thread work, ret=" << ret << " listening at " << _port << std::endl;

	if (_loop_ptr)
	{
		_loop_ptr->run();
	}
	return 0;
	/*ret = uv_run(_loop, UV_RUN_DEFAULT);
	std::cout << "socket server stop, ret = " << ret << std::endl;
	uv_loop_close(_loop);
	free(_loop);
	_loop = NULL;*/
}

void SslServer::stop()
{
	/*if (_loop)
	{
		std::cout << "stop loop" << std::endl;
		uv_stop(_loop);
		uv_async_send(&_stop_handle);
	}*/
	if (_loop_ptr)
	{
		_loop_ptr->stop();
	}
}

void SslServer::accept(uv_stream_t* server)
{
	uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	if (client == NULL)
	{
		std::cout << "on new connection, malloc client is NULL" << std::endl;
		return;
	}
	/*if (!_loop)
	{
		return;
	}*/
	int r = uv_tcp_init(_loop_ptr->uv_loop(), client);
	ASSERT(r == 0 && client->type == UV_TCP);
	r = uv_accept(server, (uv_stream_t*)client);
	ASSERT(r == 0);

	auto ptr = create_connection(client);
	client->data = ptr.get();

	r = uv_read_start((uv_stream_t*)client, ssl_alloc_buffer, ssl_after_read);
	ASSERT(r == 0);

	if (_conn_cb)
	{
		_conn_cb(ptr);
	}
}

uv_loop_t* SslServer::loop()
{
	return _loop_ptr->uv_loop();
}

std::shared_ptr<SslConnection> SslServer::create_connection(uv_tcp_t* handle)
{
	if (!handle)
	{
		return NULL;
	}
	std::shared_ptr<SslConnection> ptr = std::make_shared<SslConnection>(_loop_ptr, handle, true); 
	return ptr;
}

void SslServer::on_newconnection(SslConnectionCallBack cb)
{
	_conn_cb = cb;
}
NS_UVCORE_E