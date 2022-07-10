#ifndef UVCORE_WEBSOCKET_DEF_H
#define UVCORE_WEBSOCKET_DEF_H

#include <stdint.h>

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
	uint8_t mask_key[4];
}ws_ext_hdr;

typedef struct
{
	ws_ext_hdr hdr;
	uint8_t* arr;
}ws_t;

ws_t* unpack(const char* src, int len);

#endif