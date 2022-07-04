#include "packet_helpers.h"
#include "../utils/byte_order.hpp"
#include <string.h>

NS_UVCORE_B

//#define PACKET_HEADER_LEN 16

int PacketHelpers::unpacket_test(const uint8_t* data, int len)
{
	if (len < PACKET_HEADER_LEN)
	{
		return -1;
	}
	uint32_t payload_len = 0;
	//std::copy(data, data + 4, &payload_len);
	memcpy(&payload_len, data, 4);
	payload_len = sockets::networkToHost32(payload_len);
	if (payload_len + PACKET_HEADER_LEN > len)
	{
		return -1;
	}
	return payload_len;
}

int PacketHelpers::unpack(packet_t& packet, std::string& payload, const uint8_t* data, int len)
{
	int payload_len = unpacket_test(data, len);
	if (payload_len < 0)
	{
		return -1;
	}
	packet = *(packet_t*)(data + 4);
	packet.version = sockets::networkToHost16(packet.version);
	packet.padding = sockets::networkToHost16(packet.padding);
	packet.service_id = sockets::networkToHost16(packet.service_id);
	packet.cmd_id = sockets::networkToHost16(packet.cmd_id);
	packet.trans_id = sockets::networkToHost32(packet.trans_id);

	payload = std::string((const char*)(data + PACKET_HEADER_LEN), payload_len);
	return payload_len;
}

int PacketHelpers::unpack(packet_t& packet, const uint8_t* data, int len)
{
	int payload_len = unpacket_test(data, len);
	if (payload_len < 0)
	{
		return -1;
	}
	packet = *(packet_t*)(data + 4);
	packet.version = sockets::networkToHost16(packet.version);
	packet.padding = sockets::networkToHost16(packet.padding);
	packet.service_id = sockets::networkToHost16(packet.service_id);
	packet.cmd_id = sockets::networkToHost16(packet.cmd_id);
	packet.trans_id = sockets::networkToHost32(packet.trans_id);
	return payload_len;
}

int PacketHelpers::unpack(packet_t& packet, uint8_t* dest, int dest_len, const uint8_t* data, int len)
{
	int payload_len = unpacket_test(data, len);
	if (payload_len < 0 || dest_len < payload_len)
	{
		return -1;
	}
	packet = *(packet_t*)(data + 4);
	packet.version = sockets::networkToHost16(packet.version);
	packet.padding = sockets::networkToHost16(packet.padding);
	packet.service_id = sockets::networkToHost16(packet.service_id);
	packet.cmd_id = sockets::networkToHost16(packet.cmd_id);
	packet.trans_id = sockets::networkToHost32(packet.trans_id);

	//std::copy(data + PACKET_HEADER_LEN, data + PACKET_HEADER_LEN + payload_len, dest);
	memcpy(dest, data + PACKET_HEADER_LEN, payload_len);
	return payload_len;
}

void PacketHelpers::pack(int service_id, int cmd_id, const std::string& payload, std::string& outpack, uint32_t trans_id, uint16_t ver, uint16_t pad)
{
	pack(service_id, cmd_id, (uint8_t*)payload.c_str(), payload.size(), outpack, trans_id, ver, pad);
}

void PacketHelpers::pack(int service_id, int cmd_id, const uint8_t* data, int len, std::string& outpack, uint32_t trans_id, uint16_t ver, uint16_t pad)
{
	outpack = std::string(len + PACKET_HEADER_LEN, 0x0);

	uint32_t payload_len = len;
	payload_len = sockets::hostToNetwork32(payload_len);
	uint16_t sid = service_id;
	sid = sockets::hostToNetwork16(sid);
	uint16_t id = cmd_id;
	id = sockets::hostToNetwork16(cmd_id);
	ver = sockets::hostToNetwork16(ver);
	pad = sockets::hostToNetwork16(pad);

	uint32_t tid = trans_id;
	tid = sockets::hostToNetwork32(tid);

	char* dest = (char*)outpack.c_str();
	//std::copy(&payload_len, &payload_len + 1, dest);
	memcpy(dest, &payload_len, sizeof(payload_len));
	dest += 4;
	//std::copy(&ver, &ver + 1, dest);
	memcpy(dest, &ver, sizeof(ver));
	dest += 2;
	//std::copy(&pad, &pad + 1, dest);
	memcpy(dest, &pad, sizeof(pad));
	dest += 2;
	//std::copy(&sid, &sid + 1, dest);
	memcpy(dest, &sid, sizeof(sid));
	dest += 2;
	//std::copy(&id, &id + 1, dest);
	memcpy(dest, &id, sizeof(id));
	dest += 2;
	memcpy(dest, &tid, sizeof(tid));
	dest += 4;
	if (len > 0)
	{
		//std::copy(data, data + len, dest);
		memcpy(dest, data, len);
	}
}

NS_UVCORE_E
