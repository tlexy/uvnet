#include "global.h"

NS_UVCORE_B

Global* const Global::_instance = new Global();

Global* Global::get_instance()
{
	return _instance;
}

Global::Global()
{
	_packet_size = 1024;
	_socket_buffer = K16;
}

void Global::init()
{
	_packet_size = K4;
	_socket_buffer = K16;
}

int Global::packet_size()
{
	return _packet_size;
}

void Global::set_packet_size(size_t size)
{
	_packet_size = size;
}

void Global::set_socket_buffer(size_t size)
{
	if (size == 0 || size % K4 != 0)
	{
		return;
	}
	_socket_buffer = size;
}

int Global::socket_buffer()
{
	return _socket_buffer;
}

NS_UVCORE_E