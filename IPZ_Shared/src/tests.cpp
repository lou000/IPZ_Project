#include <iostream>
#include <thread>
#include "NetworkHandler.h"
#include "GameClient.h"
#include "GameServer.h"
#include "reference.h"
void sanity_test()
{
	UdpNetworkHandler cut(Owner::Server);
}

std::thread r;

 void recieve()
{
	asio::io_context context;
	asio::ip::udp::socket socket(context);
	asio::ip::udp::endpoint remote_endpoint;
	asio::io_context::work idleWork(context);
	//asio::executor_work_guard<asio::io_context::executor_type>;
	//auto work = asio::require(context.get_executor(), asio::execution::outstanding_work.tracked);
	//asio::any_io_executor work= asio::require(context.get_executor(), asio::execution::outstanding_work.tracked);
	socket.open(asio::ip::udp::v4());
	asio::error_code err;
	socket.bind(asio::ip::udp::endpoint(asio::ip::address::from_string("127.0.0.1"), 9000));

	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	std::vector<char> buffer(1024);


	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	r = std::thread([&] {context.run(); });
	while (true)
	{
		socket.async_receive_from(asio::buffer(buffer),
			remote_endpoint,
			[&](const asio::error_code& error, size_t bytes)
			{
				if (error) {
					std::cout << "Receive failed: " << error.message() << "\n";
					return;
				}
				std::cout << "Received: '" << std::string(buffer.begin(), buffer.begin() + bytes) << "' (" << error.message() << ")\n";

			});
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	//context.run();
	//if (socket.is_open())
	//{		
	//	while (true)
	//	{
	//	
	//		//if (bytes)
	//		//{
	//		//	std::cout << "\nGetting message";
	//		//	socket.receive_from(asio::buffer(buffer), remote_endpoint);
	//		//	for (auto x : buffer)
	//		//	{
	//		//		std::cout << x;
	//		//	}
	//		//}
	//		
	//	}
	//	std::cout << err.message();
	//	socket.close();
	//}
}

void send()
{
	asio::io_service io_service;

	asio::ip::udp::endpoint remote_endpoint;
	asio::io_context context;

	asio::error_code err;
	asio::ip::udp::socket socket(context);
	socket.open(asio::ip::udp::v4());
	remote_endpoint = asio::ip::udp::endpoint(asio::ip::make_address("127.0.0.1", err), 9000);
	socket.async_send_to(asio::buffer("Jane Doe", 8), remote_endpoint, [](const asio::error_code& error,
		std::size_t bytes_transferred)
		{
			std::cout << "Wrote " << bytes_transferred << " bytes with " <<
				error.message() << std::endl;
		});
	r = std::thread([&] {context.run(); });
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	//socket.close();
	std::cout << "\nsent\n";
}

void test_connection()
{
	char c;
	std::cin >> c;
	switch (c)
	{
		case 's':
		{
			for (int i = 0; i < 3; ++i) {
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				send();

			}
			break;
		}
		case 'r':
		{
			recieve();
			break;
		}

		default:
			break;

	}	
}


void udp_test_network_handler_connection()
{
	
	
	char c;
	std::cin >> c;
	switch (c)
	{
	case 's':
	{
		UdpNetworkHandler cut(Owner::Client);

		cut.Connect("127.0.0.1", 9000);
		for (int i = 0; i < 3; ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			//cut.Send("some message");

		}
		break;
	}
	case 'r':
	{
		UdpNetworkHandler cut(Owner::Server);

		cut.Connect("127.0.0.1", 9000);
		/* r = std::thread([cut] {cut.Listen(); });*/
		cut.Listen();
		break;
	}

	default:
		break;
	}
}

void tcp_test_network_handler_connection()
{
	TcpNetworkHandler cut(Owner::Server);
	cut.Connect("127.0.0.1", 9000);
	char c;
	std::cin >> c;
	switch (c)
	{
	case 's':
	{
		for (int i = 0; i < 3; ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			//cut.Send("some message");

		}
		break;
	}
	case 'r':
	{
		cut.Listen();
		break;
	}

	default:
		break;
	}
}

#define local "127.0.0.1"
#define remote "192.168.0.248"
#define remote1 "192.168.0.52"
void test_server_listen()
{
	GameServer serv;
	serv.Start(local,9905);
	char c;
	std::cin >> c;
	serv.Disconnect();
}

void test_client_connect()
{
	GameClient cli;
	cli.Start(remote, 9905);
	//char c;
	//std::cin >> c;
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	cli.Disconnect();

}

void test_client_server_communication()
{
	r = std::thread([&] {test_server_listen(); });
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	test_client_connect();
}

void test_reference_serv()
{
	olc::net::server_interface<int> s(9905);
	s.Start();
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}
void test_reference_cli()
{
	olc::net::client_interface<int> c;
	c.Connect("127.0.0.1", 9905);
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

int main()
{
	try
	{
		char c;
		std::cin >> c;
		switch (c)
		{
		case 'c':
			//test_reference_cli();
			//recieve();
			test_client_connect();
			std::cin >> c;
			break;
		case 's':
			//send();
			//test_reference_serv();
			test_server_listen();
			break;
		default:
			break;
		}
	}
	catch (std::exception& e)
	{
		std::cout << e.what();

	}

	r.join();
	system("pause");
	return 0;
}