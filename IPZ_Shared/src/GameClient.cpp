#include "GameClient.h"

//GameClient::GameClient() : EndpointType(Owner::Client)
//{
//}

GameClient::~GameClient()
{
	Disconnect();
}


void GameClient::Disconnect()
{
	Message msg;
	msg.header.src_id = id;
	msg.header.dest_id = 0;
	msg.header.type = MsgType::Disconnect;
	msg.header.size = 0;
	tcp.Send(msg);
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	tcp.Disconnect();
	udp.Disconnect();
}

void GameClient::OnConnection()
{
	std::cout << "Client connected with tcp\n";
	tcp.RegisterCallback(std::bind(&GameClient::OnTcpConnection, this), MsgType::Connect);
	tcp.ReadHeader();

}

void GameClient::OnTcpConnection()
{
	std::cout << "Client sending tcp hello\n";
	tcp.RegisterCallback(std::bind(&GameClient::OnTcpAccepted, this), MsgType::Connect);
	Message msg = tcp.GetLastMessage();
	id = msg.header.dest_id;
	msg.header.src_id = msg.header.dest_id;
	msg.header.dest_id = 0;
	msg.header.type = MsgType::Connect;
	tcp.Send(msg);
	tcp.ReadHeader();

}

void GameClient::OnTcpAccepted()
{
	std::cout << "Sending Hello from client\n";
	udp.Listen();
	Message msg;
	msg.header.src_id = id;
	msg.header.dest_id = 0;
	msg.header.type = MsgType::Connect;
	msg.header.size = 0;
	udp.Send(msg);

}

void GameClient::OnUdpConnection()
{
	std::cout << "Udp connection established on client\n";
}

void GameClient::OnDisconnect()
{
	std::cout << "Server sent disconnection message";
	tcp.Disconnect();
	udp.Disconnect();
}

void GameClient::OnTcpMsg()
{
}

void GameClient::OnUdppMsg()
{
}

void GameClient::Start(std::string address, unsigned short port)
{
	tcp.RegisterCallback(std::bind(&GameClient::OnConnection, this), MsgType::Connect);
	tcp.RegisterCallback(std::bind(&GameClient::OnDisconnect, this), MsgType::Disconnect);
	udp.RegisterCallback(std::bind(&GameClient::OnUdpConnection, this), MsgType::Connect);
	tcp.Connect(address, port);
	udp.Connect(address, port);
}

