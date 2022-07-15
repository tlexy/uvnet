#ifndef UVCORE_WEBSOCKET_CHANNEL_H
#define UVCORE_WEBSOCKET_CHANNEL_H

#include <websocket/websocket.h>
#include <string>

class WebsocketChannel
{
public:
	void send_ping(const std::string&);
	void send_pong(const std::string&);
	void send_close();

	//传入的数据是websocket格式
	virtual void send_ws_message(const char* data, int len) = 0;

	//原始格式
	virtual int writeInLoop(const char* data, int len, OpCode) = 0;
	virtual int write(const char* data, int len, OpCode) = 0;

	void send_ws_content(OpCode op, const std::string&);
};

#endif