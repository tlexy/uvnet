#include <websocket/ws_connection.h>
#include <core/event_loop.h>
#include <iostream>
#include <httpparser/request.h>
#include <httpparser/httprequestparser.h>
#include <httpparser/response.h>
#include <utils/endec.h>

NS_UVCORE_B

const static std::string magic_code = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

WsConnection::WsConnection(std::shared_ptr<EventLoop> loop, uv_tcp_t* handle, bool del)
	:TcpConnection(loop, handle, del)
{
}

WsConnection::~WsConnection()
{
}

void WsConnection::err_close(int error)
{
	set_error(error);
	TcpConnection::on_receive_data(0);
}

int WsConnection::get_version(std::shared_ptr<httpparser::Request> req)
{
	std::string sv = get_header_string(req, "Sec-WebSocket-Version");
	return std::atoi(sv.c_str());
}

std::string WsConnection::get_header_string(std::shared_ptr<httpparser::Request> req, const std::string& key)
{
	for (int i = 0; i < req->headers.size(); ++i)
	{
		if (req->headers[i].name == key)
		{
			return req->headers[i].value;
		}
	}
	return std::string("");
}

void WsConnection::on_receive_data(size_t len)
{
	if (!is_handshake())
	{
		do_handshake();
	}
	else
	{
		handle_ws_data_frame();
	}
}

void WsConnection::do_handshake()
{
	using namespace httpparser;

	const char* text = reinterpret_cast<const char*>(get_inner_buffer()->read_ptr());
	int len = get_inner_buffer()->readable_size();

	auto request = std::make_shared<Request>();
	HttpRequestParser parser;
	HttpRequestParser::ParseResult res = parser.parse(*request, text, text + len);
	if (res == HttpRequestParser::ParsingCompleted)
	{
		get_inner_buffer()->has_read(len);
		std::cout << request->inspect() << std::endl;
		if (request->versionMajor != 1
			|| request->method != "GET"
			|| get_version(request) != 13
			|| get_header_string(request, "Connection") != std::string("Upgrade")
			|| get_header_string(request, "Upgrade") != std::string("websocket"))
		{
			err_close(HandshakeError);
		}
		std::string key = get_header_string(request, "Sec-WebSocket-Key");
		std::string text = key + magic_code;
		std::string sha = sha1(text);
		std::string val = base64Encode(sha.c_str(), sha.size());

		httpparser::Response resp;
		resp.status = "Switching Protocols";
		resp.versionMajor = 1;
		resp.versionMinor = 1;
		resp.keepAlive = true;
		resp.statusCode = 101;
		resp.headers["Upgrade"] = "websocket";
		resp.headers["Connection"] = "Upgrade";
		resp.headers["Sec-WebSocket-Accept"] = val;

		std::string resp_text = resp.inspect();
		TcpConnection::write(resp_text.c_str(), resp_text.size());

		_is_handshake = true;
	}
	else if (res == HttpRequestParser::ParsingError)
	{
		err_close(1);
	}
	else
	{
		//std::cerr << "Http wait more data for parsing." << std::endl;
	}
}

void WsConnection::handle_ws_data_frame()
{
	int a = 1;
}

int WsConnection::write(const char* data, int len)
{
	return 0;
}

int WsConnection::writeInLoop(const char* data, int len)
{
	return 0;
}

bool WsConnection::is_handshake()
{
	return _is_handshake;
}

void WsConnection::set_handshake_cb(HandshakeCallBack cb)
{
	_hcb = cb;
}

//void WsConnection::set_receive_cb(DataCallBack cb)
//{}
//
//void WsConnection::set_close_cb(CloseCallBack cb)
//{}

NS_UVCORE_E