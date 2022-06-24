#ifndef UVCORE_IP_ADDRESS_H
#define UVCORE_IP_ADDRESS_H

#include "../utils/ns_helper.h"
#include <sys/types.h>
#ifndef _WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <WS2tcpip.h>
#include <windows.h>
#endif
#include <string>

NS_UVCORE_B

/*
* currently only support IPv4, it also can be extend to support IPv6
*/

class IpAddress
{
public:
	IpAddress(bool valid = false);
	IpAddress(int port, int family = AF_INET);
	//void setIp(uint32_t ip = INADDR_ANY, int family = AF_INET);
	void setIp(const std::string&, int family = AF_INET);
	void setPort(unsigned short port);
	//unsigned short port() const { return ntohs(_addr.addr_ip4.sin_port); }
	struct sockaddr* rawAddressPtr() const { return (struct sockaddr*)&_addr; }
	std::string toString(bool withPort = true) const;
	int addressLength();
	int family() { return _addr.addr_ip4.sin_family; }
	static IpAddress fromRawSocketAddress(sockaddr*, socklen_t);
	void set_valid(bool valid = true);
	std::string getIp();
	int getPort();

	void set_doname(const std::string& doname);
	void update_addr();

	bool operator()() const { return _valid; }

private:
	union
	{
		struct sockaddr_in addr_ip4;
		struct sockaddr_in6 addr_ip6;
	}_addr;
	bool _valid{false};
	std::string _doname;
	std::string _ip_str;
	char _ipbuf[16];
	int _port;
};

NS_UVCORE_E

#endif
