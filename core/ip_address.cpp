#include "ip_address.h"
#include <stdlib.h>
#include <string.h>
#include "../utils/sock_utils.h"
//#include <iostream>

#define IPV4_LEN 16

NS_UVCORE_B

IpAddress::IpAddress(bool valid)
	:_valid(valid)
{
	memset(&_addr, 0x0, sizeof(_addr));
	_addr.addr_ip4.sin_family = AF_INET;
}

IpAddress::IpAddress(int port, int family)
	:_valid(true)
{
	//std::cout << "family:" << family << std::endl;
	_port = port;
	memset(&_addr, 0x0, sizeof(_addr));
	_addr.addr_ip4.sin_family = family;
	_addr.addr_ip4.sin_port = htons(port);
}

//void IpAddress::setIp(uint32_t ip, int family)
//{
//	if (family == AF_INET)
//	{
//		_addr.addr_ip4.sin_addr.s_addr = htonl(ip);
//	}
//}

void IpAddress::setIp(const std::string& ip_str, int family)
{
	_ip_str = ip_str;
	if (family == AF_INET)
	{
		inet_pton(AF_INET, ip_str.c_str(), &_addr.addr_ip4.sin_addr.s_addr);
	}
}

void IpAddress::setPort(unsigned short  port)
{
	_port = port;
	_addr.addr_ip4.sin_port = htons(port);
}

std::string IpAddress::toString(bool withPort) const
{
	if (_addr.addr_ip4.sin_family != AF_INET)
	{
		return std::string();
	}
	/*if (_ip_str.size() < 1)
	{
		char buf[IPV4_LEN];
		memset(buf, 0x0, sizeof(buf));
		inet_ntop(AF_INET, (void*)&_addr.addr_ip4.sin_addr.s_addr, (char*)&buf, IPV4_LEN);
		_ip_str = std::string((const char*)&buf);
	}*/
	std::string ip = _ip_str;
	if (withPort)
	{
		//if (_port < 0)
		//{
		//	_port = ntohs(_addr.addr_ip4.sin_port);
		//}
		ip.append(":");
		ip.append(std::to_string(_port));
	}

	return ip;
}

void IpAddress::parse()
{
	char buf[IPV4_LEN];
	memset(buf, 0x0, sizeof(buf));
	inet_ntop(AF_INET, (void*)&_addr.addr_ip4.sin_addr.s_addr, (char*)&buf, IPV4_LEN);
	_ip_str = std::string((const char*)&buf);

	_port = ntohs(_addr.addr_ip4.sin_port);
}

std::string IpAddress::getIp()
{
	return _ip_str;
}

int IpAddress::getPort()
{
	return _port;
}

void IpAddress::set_doname(const std::string& doname)
{
	_doname = doname;
}

void IpAddress::update_addr()
{
	if (_doname.size() > 2)
	{
		memset(_ipbuf, 0x0, sizeof(_ipbuf));
		in_addr_t saddr = sockets::getIpaddrByName(_doname.c_str(), _ipbuf, sizeof(_ipbuf));
		if (saddr != INADDR_NONE)
		{
			_ip_str = std::string(_ipbuf, strlen(_ipbuf));
		}
	}
}

int IpAddress::addressLength()
{
	if (_addr.addr_ip4.sin_family == AF_INET)
	{
		return sizeof(_addr.addr_ip4);
	}
	return 0;
}

void IpAddress::set_valid(bool valid)
{
	_valid = valid;
}

IpAddress IpAddress::fromRawSocketAddress(sockaddr* sa, socklen_t len)
{
	IpAddress ip;
	memcpy(ip.rawAddressPtr(), sa, len);
	ip.set_valid();
	ip.parse();
	return ip;
}

NS_UVCORE_E