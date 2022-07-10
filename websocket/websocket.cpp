#include <websocket/websocket.h>
#include <utils/byte_order.hpp>

ws_t* unpack(const char* src, int len)
{
	if (len < sizeof(ws_hdr))
	{
		return nullptr;
	}
	ws_t* ws = new ws_t;
	ws_hdr* hdr = (ws_hdr*)src;
	int ext_pl = 0;
	if (hdr->payload_len < 126)
	{
		ws->arr = new uint8_t[hdr->payload_len];
	}
	else if (hdr->payload_len == 126)
	{
		if (len < 4)
		{
			delete ws;
			return nullptr;
		}
		ext_pl = 2;
		uint16_t* pl = (uint16_t*)(src + 2);
		ws->hdr.payload_len = sockets::networkToHost16(*pl);
	}
	else if (hdr->payload_len == 127)
	{
		if (len < 10)
		{
			delete ws;
			return nullptr;
		}
		ext_pl = 8;
		uint64_t* pl = (uint64_t*)(src + 2);
		ws->hdr.payload_len = sockets::networkToHost64(*pl);
	}
	if (hdr->mask == 1)
	{
		if (len < 2 + ext_pl + 4)
		{
			delete ws;
			return nullptr;
		}

	}
}