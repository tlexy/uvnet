#include <websocket/websocket.h>
#include <utils/byte_order.hpp>
#include <string.h>
#include <stdlib.h>

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
		ws->hdr.payload_len = hdr->payload_len;
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
		ws->hdr.mask_key[3] = src[2 + ext_pl + 3];
		ws->hdr.mask_key[2] = src[2 + ext_pl + 2];
		ws->hdr.mask_key[1] = src[2 + ext_pl + 1];
		ws->hdr.mask_key[0] = src[2 + ext_pl];

		ext_pl += 4;
	}

	if (len < 2 + ext_pl + ws->hdr.payload_len)
	{
		delete ws;
		return nullptr;
	}
	if (ws->hdr.payload_len > 0)
	{
		ws->arr = new uint8_t[ws->hdr.payload_len];
		memcpy(ws->arr, src + 2 + ext_pl, ws->hdr.payload_len);
	}
	ws->hdr.hdr = *hdr;
	ws->hdr.ext_len = ext_pl;
	if (ws->hdr.hdr.mask)
	{
		unmask(ws->arr, ws->hdr.payload_len, ws->hdr.mask_key);
	}
	return ws;
}

void unmask(uint8_t* data, int len, const uint8_t* mask)
{
	for (int i = 0; i < len; ++i)
	{
		data[i] = data[i] ^ mask[i % 4];
	}
}

void free_ws(ws_t* ws)
{
	if (ws->hdr.payload_len > 0)
	{
		delete[] ws->arr;
	}
	delete ws;
}

ws_t* pack(const char* data, int len, OpCode)
{
	return nullptr;
}

int pack_len(ws_t* ws)
{
	int len = sizeof(ws_hdr);
	if (ws->hdr.hdr.payload_len == 126)
	{
		len += 2;
	}
	else if (ws->hdr.hdr.payload_len == 127)
	{
		len += 8;
	}

	if (ws->hdr.hdr.mask == 1)
	{
		len += 4;
	}
	return len + ws->hdr.payload_len;
}

int pack_len(int len)
{
	int ext_len = 0;
	if (len <= 125)
	{
		
	}
	else if (len >= 126 && len < 65535)
	{
		ext_len = 2;
	}
	else
	{
		ext_len = 8;
	}
	return sizeof(ws_hdr) + ext_len + len;
}

bool pack_and_copy(const char* data, int len, OpCode op, void* dest, int dest_len)
{
	ws_hdr hdr;
	memset(&hdr, 0x0, sizeof(ws_hdr));
	hdr.opcode = op;
	hdr.fin = 1;

	int ext_len = 0;
	if (len <= 125)
	{
		hdr.payload_len = len;
	}
	else if (len >= 126 && len < 65535)
	{
		ext_len = 2;
		hdr.payload_len = 126;
	}
	else
	{
		ext_len = 8;
		hdr.payload_len = 127;
	}
	if (sizeof(ws_hdr) + ext_len + len > dest_len)
	{
		return false;
	}

	char* p = (char*)dest;
	memcpy(p, &hdr, sizeof(hdr));
	p += 2;
	if (hdr.payload_len == 126)
	{
		uint16_t plen = len;
		plen = sockets::hostToNetwork16(plen);
		memcpy(p, &plen, sizeof(plen));
		p += (sizeof(uint16_t));
	}
	else if (hdr.payload_len == 127)
	{
		uint64_t plen = len;
		plen = sockets::hostToNetwork64(plen);
		memcpy(p, &plen, sizeof(plen));
		p += (sizeof(uint64_t));
	}
	if (len > 0)
	{
		memcpy(p, data, len);
	}
	return true;
}
