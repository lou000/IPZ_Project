#include "GameServer.h"
//GameServer::GameServer(): EndpointType(Owner::Server)
//{
//
//}

void GameServer::Disconnect()
{
	Message msg;
	msg.header.src_id = 0;
	msg.header.dest_id = 0;
	msg.header.type = MsgType::Disconnect;
	msg.header.dest = MsgDestination::Multicast;
	msg.header.size = 0;
	tcp.Send(msg);
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	udp.Disconnect();
	tcp.Disconnect();
}

void GameServer::Start(std::string addres, unsigned short port)
{
	this->port = port;
	tcp.RegisterCallback(std::bind(&GameServer::OnTcpMsg, this), MsgType::Update);
	tcp.RegisterCallback(std::bind(&GameServer::OnDisconnect, this), MsgType::Disconnect);
	tcp.RegisterCallback(std::bind(&GameServer::OnConnection, this), MsgType::Connect);
	udp.RegisterCallback(std::bind(&GameServer::OnUdpConnection, this), MsgType::Connect);
	tcp.Connect("127.0.0.1", port);
	tcp.Listen();
	udp.Connect("127.0.0.1", port);
	udp.Listen();

}

void GameServer::OnConnection()
{
	std::cout << "TCP connected\n";
	tcp.RegisterCallback(std::bind(&GameServer::OnTcpConnection, this), MsgType::Connect);
	Message msg;
	msg.header.src_id = 0;
	msg.header.dest = MsgDestination::Client;
	msg.header.dest_id = tcp.getMostRecentConnectionId();
	msg.header.type = MsgType::Connect;
	msg.header.size = 0;
	tcp.Send(msg);
	tcp.ReadHeader();
}

void GameServer::OnTcpConnection()
{
	//tcp.Send("Accepted");
	std::cout << "tcp accepted\n";
	auto endpoint = tcp.GetEndPoint();
	udp.AddEndpoint(asio::ip::udp::endpoint(endpoint.address(),port));
	Message msg = tcp.GetLastMessage();
	msg.header.dest_id = msg.header.src_id;
	msg.header.dest = MsgDestination::Client;
	msg.header.src_id = 0;
	msg.header.type = MsgType::Connect;
	tcp.Send(msg);
	tcp.RegisterCallback(std::bind(&GameServer::OnConnection, this), MsgType::Connect);
	tcp.ReadHeader();
	udp.ReadHeader();

}

void GameServer::OnUdppMsg()
{
}

void GameServer::OnTcpMsg()
{
	std::cout << "Got tcp message from client\n";
	Message msg = tcp.GetLastMessage();
	std::cout << msg;
	char m[256];
	msg >> m;
	//std::cout <<"Data is "<< m;
	for (char letter : msg.body)
	{
		std::cout << letter;
	}
}

void GameServer::OnUdpConnection()
{
	std::cout << "udp connected on client\n";
	Message msg = tcp.GetLastMessage();
	msg.header.dest_id = msg.header.src_id;
	msg.header.src_id = 0;
	udp.Send(msg);
}

void GameServer::OnDisconnect()
{
	Message msg = tcp.GetLastMessage();
	std::cout << "Client "<<msg.header.src_id<<" sent disconnection message";
	tcp.RemoveConnectionAt(msg.header.src_id);
	udp.RemoveConnectionAt(msg.header.src_id);
}
