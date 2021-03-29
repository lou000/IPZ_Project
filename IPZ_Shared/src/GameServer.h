#pragma once
#include "EndpointType.h"
class GameServer : public EndpointType
{
public:
	GameServer() : EndpointType(Owner::Server) {}
	~GameServer()
	{
		Disconnect();
	}
	void Start(std::string addres, unsigned short port) override;
	void Disconnect() override;
private:
	void OnConnection() override;
	void OnTcpConnection() override;
	void OnUdppMsg() override;
	void OnTcpMsg() override;
	void OnUdpConnection() override;
	void OnDisconnect() override;
};

