#include <tls/ssl_ws_connection.h>
#include <core/event_loop.h>
#include <iostream>
#include <httpparser/request.h>
#include <httpparser/httprequestparser.h>
#include <httpparser/response.h>
#include <utils/endec.h>
#include <websocket/websocket.h>
#include <utils/global.h>

NS_UVCORE_B

const static std::string magic_code = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

SslWsConnection::SslWsConnection(std::shared_ptr<EventLoop> loop, uv_tcp_t* handle, bool del)
	:TcpConnection(loop, handle, del),
	_dec_buffer(Global::get_instance()->socket_buffer()),
	_ssl_dec_buffer(Global::get_instance()->socket_buffer())
{
}

SslWsConnection::~SslWsConnection()
{
}

CircleBuffer* SslWsConnection::get_dec_buffer()
{
	return &_dec_buffer;
}

void SslWsConnection::err_close(int error)
{
	set_error(error);
	TcpConnection::on_receive_data(0);
}

void SslWsConnection::write_socket(const char* data, int len)
{
	TcpConnection::write(data, len);
}

int SslWsConnection::get_version(std::shared_ptr<httpparser::Request> req)
{
	std::string sv = get_header_string(req, "Sec-WebSocket-Version");
	return std::atoi(sv.c_str());
}

std::string SslWsConnection::get_header_string(std::shared_ptr<httpparser::Request> req, const std::string& key)
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

void SslWsConnection::on_receive_data(size_t len)
{
	while (get_inner_buffer()->readable_size() > 0)
	{
		write_to_bio(get_inner_buffer());
		if (!is_handshake_finished())
		{
			if (do_ssl_handshake() == SSLSTATUS_FAIL)
			{
				//握手时出错了
				std::cerr << "SSLSTATUS_FAIL" << std::endl;
				//SSL_clear(_ssl);
				close();
				return;
			}
			if (!is_handshake_finished())
			{
				return;
			}
			else
			{
				if (_ncb)
				{
					_ncb(std::dynamic_pointer_cast<SslWsConnection>(shared_from_this()));
				}
			}
		}
	}

	read_from_bio(get_ssl_dec_buffer());
	if (!is_handshake())
	{
		do_ws_handshake();
	}
	else
	{
		handle_ws_data_frame();
	}
}

void SslWsConnection::do_ws_handshake()
{
	using namespace httpparser;

	const char* text = reinterpret_cast<const char*>(get_ssl_dec_buffer()->read_ptr());
	int len = get_ssl_dec_buffer()->readable_size();

	auto request = std::make_shared<Request>();
	HttpRequestParser parser;
	HttpRequestParser::ParseResult res = parser.parse(*request, text, text + len);
	if (res == HttpRequestParser::ParsingCompleted)
	{
		get_ssl_dec_buffer()->has_read(len);
		//std::cout << request->inspect() << std::endl;
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

		std::string resp_text = resp.inspect2();
		write(resp_text.c_str(), resp_text.size(), false);

		_is_handshake = true;
		if (_hcb)
		{
			_hcb(std::dynamic_pointer_cast<SslWsConnection>(shared_from_this()));
		}
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

void SslWsConnection::handle_ws_data_frame()
{
	while (true)
	{
		auto ws = unpack((const char*)get_ssl_dec_buffer()->read_ptr(), get_ssl_dec_buffer()->readable_size());
		if (ws == nullptr)
		{
			break;
		}
		else
		{
			get_ssl_dec_buffer()->has_read(ws->hdr.ext_len + ws->hdr.payload_len + 2);
		}
		
		if (ws->hdr.hdr.opcode == WsTextFrame || ws->hdr.hdr.opcode == WsBinaryFrame)
		{
			get_dec_buffer()->write(ws->arr, ws->hdr.payload_len);
			if (ws->hdr.hdr.fin == 1)
			{
				TcpConnection::on_receive_data(ws->hdr.payload_len);
			}
		}
		else if (ws->hdr.hdr.opcode == WsConnectionClose)
		{
			if (_wsccb)
			{
				_wsccb(std::dynamic_pointer_cast<SslWsConnection>(shared_from_this()), std::string((const char*)ws->arr, ws->hdr.payload_len));
			}
		}
		else if (ws->hdr.hdr.opcode == WsPing)
		{
			if (_wspcb)
			{
				_wspcb(std::dynamic_pointer_cast<SslWsConnection>(shared_from_this()), std::string((const char*)ws->arr, ws->hdr.payload_len));
			}
		}
		else
		{
			std::cerr << "unsupported opcode: " << ws->hdr.hdr.opcode << std::endl;
		}

		free_ws(ws);
	}
	int a = 1;
}

int SslWsConnection::write(const char* data, int len, bool is_ws)
{
	int ori_len = len;
	SSLStatus status;
	if (is_ws)
	{
		int buff_len = pack_len(len);
		char* buff = (char*)malloc(buff_len);
		if (buff == NULL)
		{
			return 2;
		}
		pack_and_copy(data, len, WsTextFrame, buff, buff_len);
		ori_len = buff_len;
		//std::cout << "SslWsConnection::write， pre data len = " << _raw_write_buffer.readable_size() << std::endl;
		status = write_to_ssl(buff, buff_len);
		free(buff);
	}
	else
	{
		status = write_to_ssl(data, len);
	}
	if (status == SSLSTATUS_FAIL)
	{
		return -1;
	}
	//std::cout << "SslWsConnection::write, data len = " << _raw_write_buffer.readable_size() << ", ori_len: " << ori_len << std::endl;
	int ret = TcpConnection::write((const char*)_raw_write_buffer.read_ptr(), _raw_write_buffer.readable_size());
	_raw_write_buffer.reset();
	//int ret = TcpConnection::write(buff, buff_len);;
	return ret;
}

int SslWsConnection::writeInLoop(const char* data, int len)
{
	if (_loop_ptr->isRunInLoopThread())
	{
		return write(data, len);
	}
	else
	{
		WriteReq* req = new WriteReq;
		req->req.data = this;
		int buff_len = pack_len(len);
		char* buff = (char*)malloc(buff_len);
		if (buff == NULL)
		{
			return 2;//内存分配失败
		}
		pack_and_copy(data, len, WsTextFrame, buff, buff_len);
		auto status = write_to_ssl(buff, buff_len);
		free(buff);
		if (status == SSLSTATUS_FAIL)
		{
			return -1;
		}
		req->buf = uv_buf_init((char*)_raw_write_buffer.read_ptr(), _raw_write_buffer.readable_size());
		_raw_write_buffer.reset();
		_loop_ptr->runInLoop(std::bind(&SslWsConnection::async_write, this, req));
	}
	return 0;
}

bool SslWsConnection::is_handshake()
{
	return _is_handshake;
}

void SslWsConnection::set_handshake_cb(HandshakeCallBack cb)
{
	_hcb = cb;
}

void SslWsConnection::set_ws_close_cb(HandleWsCloseCallBack cb)
{
	_wsccb = cb;
}

void SslWsConnection::set_ws_ping_cb(HandlePingCallBack cb)
{
	_wspcb = cb;
}

void SslWsConnection::set_new_ssl_cb(SslHandshakeCallBack cb)
{
	_ncb = cb;
}

CircleBuffer* SslWsConnection::get_ssl_dec_buffer()
{
	return &_ssl_dec_buffer;
}

NS_UVCORE_E