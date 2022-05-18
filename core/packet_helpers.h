#ifndef UVCORE_PACKET_HELPERS_H
#define UVCORE_PACKET_HELPERS_H

#include "../utils/ns_helper.h"
#include <stdint.h>
#include <string>

/*
	通用数据包格式： 
	|len(4B)|ver(2B)|padding(2B)|service_id(2B)|cmd_id(2B)|trans_id(4B)|payload(len)|
	这里len的长度为payload的长度，不包括头部。
	解析：
		1. 是否足够16个字节
		2. 获取payload长度
		3. 判断是否是一个完整的包
*/

NS_UVCORE_B

#pragma pack(push)
#pragma pack(2)

typedef struct
{
	uint16_t version;
	uint16_t padding;
	uint16_t service_id;
	uint16_t cmd_id;
	uint32_t trans_id;//追踪ID
}packet_t;

#pragma pack(pop)

class PacketHelpers
{
public:
	static int unpacket_test(const uint8_t*, int len);
	static int unpack(packet_t& packet, std::string& payload, const uint8_t* data, int len);
	static int unpack(packet_t& packet, uint8_t* dest, int dest_len, const uint8_t* data, int len);
	static void pack(int service_id, int cmd_id, const std::string&, std::string&, uint32_t trans_id = 0, uint16_t ver = 0, uint16_t pad = 0);
	static void pack(int service_id, int cmd_id, const uint8_t*, int len, std::string&, uint32_t trans_id = 0, uint16_t ver = 0, uint16_t pad = 0);
};

NS_UVCORE_E

#endif