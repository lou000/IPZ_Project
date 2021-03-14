#include "NetworkHandler.h"
#include <iostream>
#include <thread>
void sanity_test()
{
	UdpNetworkHandler cut(Owner::Server);
}



void recieve()
{
	asio::io_context context;
	asio::io_service io_service;
	asio::ip::udp::socket socket(context);
	asio::ip::udp::endpoint remote_endpoint;

	socket.open(asio::ip::udp::v4());
	asio::error_code err;
	socket.bind(asio::ip::udp::endpoint(asio::ip::address::from_string("127.0.0.1"), 9000));

	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	std::vector<char> buffer(1024);
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

	context.run();
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
	asio::ip::udp::socket socket(io_service);
	asio::ip::udp::endpoint remote_endpoint;
	asio::io_context context;
	socket.open(asio::ip::udp::v4());
	asio::error_code err;

	remote_endpoint = asio::ip::udp::endpoint(asio::ip::make_address("127.0.0.1", err), 9000);
	socket.send_to(asio::buffer("Jane Doe", 8), remote_endpoint, 0, err);
	socket.close();
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


void test_network_handler_connection()
{
	UdpNetworkHandler cut(Owner::Server);
	cut.Connect("127.0.0.1", 9000);
	char c;
	std::cin >> c;
	switch (c)
	{
	case 's':
	{
		for (int i = 0; i < 3; ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			cut.Send("some message");

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

int main()
{
	
	sanity_test();
	//test_connection();
	test_network_handler_connection();
	system("pause");
	return 0;
}