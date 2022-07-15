#include <websocket/websocket_channel.h>

void WebsocketChannel::send_ping(const std::string& text)
{
	send_ws_content(WsPing, text);
}

void WebsocketChannel::send_pong(const std::string& text)
{
	send_ws_content(WsPong, text);
}

void WebsocketChannel::send_ws_content(OpCode op, const std::string& text)
{
	int len = text.size();
	int buff_len = pack_len(len);
	char* buff = (char*)malloc(buff_len);
	if (buff == NULL)
	{
		return;
	}
	pack_and_copy(text.c_str(), len, op, buff, buff_len);
	send_ws_message(buff, buff_len);
	free(buff);
}

void WebsocketChannel::send_close()
{
	send_ws_content(WsConnectionClose, "");
}