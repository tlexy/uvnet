#pragma once

#ifndef UVCORE_GLOBAL_H
#define UVCORE_GLOBAL_H

#include <stdint.h>
#include <stddef.h>
#include "ns_helper.h"

#define K4 (4*1024)
#define K16 (4*K4)

NS_UVCORE_B

class Global
{
public:
	Global();
	static Global* const _instance;
	static Global* get_instance();
	void init();
	int packet_size();
	void set_packet_size(size_t size);

	void set_socket_buffer(size_t size);
	int socket_buffer();

private:
	int _packet_size{1024};
	int _socket_buffer{4096};
};

NS_UVCORE_E

#endif
