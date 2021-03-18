#pragma once
#include "NetworkHandler.h"
class EndpointType
{
public:

	EndpointType(Owner owner): port(0), udp(owner), tcp(owner){}
	virtual ~EndpointType() 
	{

	}
	virtual void Start(std::string address, unsigned short port) = 0;
	virtual void Disconnect() = 0;
protected:
	virtual void OnConnection() = 0;
	virtual void OnTcpConnection() = 0;
	virtual void OnTcpMsg() = 0;
	virtual void OnUdppMsg() = 0;
	virtual void OnUdpConnection() = 0;
	virtual void OnDisconnect() = 0;

	unsigned short port;
	UdpNetworkHandler udp;
	TcpNetworkHandler tcp;
};

