#ifndef UVCORE_WEBSOCKET_DEF_H
#define UVCORE_WEBSOCKET_DEF_H

#include <stdint.h>

enum OpCode
{
	WsNoWebsocket = 0x0,
	WsTextFrame = 0x01,
	WsBinaryFrame = 0x02,
	WsConnectionClose = 0x08,
	WsPing = 0x09,
	WsPong = 0x10
};

typedef struct websocket_header
{
	uint16_t opcode : 4;
	uint16_t rsv3 : 1;
	uint16_t rsv2 : 1;
	uint16_t rsv1 : 1;
	uint16_t fin : 1;
	uint16_t payload_len : 7;
	uint16_t mask : 1;
}ws_hdr;

typedef struct websocket_ext_header
{
	ws_hdr hdr;
	uint64_t payload_len;
	uint8_t ext_len;
	uint8_t mask_key[4];
}ws_ext_hdr;

typedef struct
{
	ws_ext_hdr hdr;
	uint8_t* arr;
}ws_t;

ws_t* unpack(const char* src, int len);
void free_ws(ws_t*);
void unmask(uint8_t* data, int len, const uint8_t* mask);

ws_t* pack(const char* data, int len, OpCode);
int pack_len(ws_t*);
int pack_len(int len);

//no mask...
bool pack_and_copy(const char* data, int len, OpCode op, void* dest, int dest_len);

#endif