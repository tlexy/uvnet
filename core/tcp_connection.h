#ifndef UVCORE_TCP_CONNECTION_H
#define UVCORE_TCP_CONNECTION_H

#include "../utils/ns_helper.h"
#include <uv.h>
#include "../utils/circle_buffer.h"
#include <functional>
#include <memory>
#include "../utils/ngenerator.hpp"

NS_UVCORE_B

struct WriteReq
{
	uv_write_t req;
	uv_buf_t buf;
};

class Packer;
class TcpConnection;
class EventLoop;

typedef std::function<void(std::shared_ptr<TcpConnection>)> DataCallBack;
typedef std::function<void(std::shared_ptr<TcpConnection>)> CloseCallBack;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
	TcpConnection(std::shared_ptr<EventLoop> loop, uv_tcp_t* handle, bool del = true);
	~TcpConnection();

	int64_t id() { return _connid; }

	void calc_ip();

	uint32_t get_remote_ip();

	char* get_buffer();
	uint32_t get_buffer_length();

	CircleBuffer* get_inner_buffer();

	bool del_handle() const;

	void set_error(int);
	int error();
	//与上层业务相关的state
	void set_state(int);
	int state();

	void set_create_time(int64_t);
	int64_t create_time();

	void has_written(size_t len);
	void on_receive_data(size_t len);

	void set_receive_cb(DataCallBack cb);
	void set_close_cb(CloseCallBack cb);

	void close();

	//怎么发送数据？
	//只能在loop线程中调用
	int write(const char* data, int len);
	//可以在任意线程中调用
	int writeInLoop(const char* data, int len);

	void del_after_write();

	void on_close();

	static void write_cb(uv_write_t* preq, int status);

private:
	int async_write(WriteReq*);

private:
	uv_tcp_t* _handle{ NULL };
	std::shared_ptr<EventLoop> _loop_ptr;
	int64_t _connid;
	CircleBuffer _buffer;
	DataCallBack _cb;
	CloseCallBack _ccb;
	bool _is_close{false};
	bool _del_after_write{false};
	static NGenerator<int64_t> _gentor;
	bool _handle_del{ true };//是否在connection关闭时free uv_tcp_t的内存
	int _error{0};
	int _state{ 0 };
	uint32_t _remote_ip{ 0 };
	int64_t _create_time;

};

NS_UVCORE_E

#endif