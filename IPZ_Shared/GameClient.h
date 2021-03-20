#pragma once
#include "EndpointType.h"
class GameClient : public EndpointType
{
public:
	GameClient() : EndpointType(Owner::Client) {}
	~GameClient();
	void Start(std::string address, unsigned short port) override;
	void Disconnect() override;
private:
	void OnConnection() override;
	void OnTcpConnection() override;
	void OnUdpConnection() override;
	void OnDisconnect() override;
	void OnTcpMsg() override;
	void OnUdppMsg() override;

	void OnTcpAccepted();

	int id;
};

