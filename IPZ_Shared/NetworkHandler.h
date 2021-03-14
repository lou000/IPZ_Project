#pragma once
#include <thread>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>


template <typename T>
struct Message
{
	struct Header
	{
		int id;
		int size;
	};
	// Header & Body vector
	Header header;
	std::vector<uint8_t> body;

	// returns size of entire message packet in bytes
	size_t size() const
	{
		return body.size();
	}

	friend std::ostream& operator << (std::ostream& os, const Message<T>& msg)
	{
		os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
		return os;
	}


	// Pushes any POD-like data into the message buffer
	template<typename DataType>
	friend Message<T>& operator << (Message<T>& msg, const DataType& data)
	{
		// Check that the type of the data being pushed is trivially copyable
		static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

		// Cache current size of vector, as this will be the point we insert the data
		size_t i = msg.body.size();

		// Resize the vector by the size of the data being pushed
		msg.body.resize(msg.body.size() + sizeof(DataType));

		// Physically copy the data into the newly allocated vector space
		std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

		// Recalculate the message size
		msg.header.size = msg.size();

		// Return the target message so it can be "chained"
		return msg;
	}

	// Pulls any POD-like data form the message buffer
	template<typename DataType>
	friend Message<T>& operator >> (Message<T>& msg, DataType& data)
	{
		// Check that the type of the data being pushed is trivially copyable
		static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pulled from vector");

		// Cache the location towards the end of the vector where the pulled data starts
		size_t i = msg.body.size() - sizeof(DataType);

		// Physically copy the data from the vector into the user variable
		std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

		// Shrink the vector to remove read bytes, and reset end position
		msg.body.resize(i);

		// Recalculate the message size
		msg.header.size = msg.size();

		// Return the target message so it can be "chained"
		return msg;
	}
};

enum class Owner
{
	Server,
	Client
};


template<typename T, typename N>
class NetworkHandler
{
public:

	NetworkHandler(Owner owner) : owner(owner), running(false)
	{
		socket = std::make_unique<T>(T(context));
	}
	void Connect(std::string address, int port);
	void Send(std::string message);
	void Listen();
private:
	
	void SetupListen();

	asio::io_service ioService;
	std::unique_ptr<T> socket;
	N remoteEndpoint;
	asio::io_context context;
	Owner owner;
	std::array<char,1024> buffer;
	std::thread contextThread;
	bool running;
};
template<typename T, typename N>
void NetworkHandler<T, N>::Connect(std::string addr, int port)
{
	remoteEndpoint.address(asio::ip::address::from_string(addr));
	remoteEndpoint.port(port);
	socket->open(asio::ip::udp::v4());
}
template<typename T, typename N>
void NetworkHandler<T, N>::Send(std::string message)
{

	asio::error_code err;
	socket->async_send_to(asio::buffer(message, message.size()), remoteEndpoint, [](const asio::error_code& error,
		std::size_t bytes_transferred)
		{
			std::cout << "Wrote " << bytes_transferred << " bytes with " <<
				error.message() << std::endl;
		});

}

template<typename T, typename N>
void NetworkHandler<T, N>::Listen()
{
	socket->bind(remoteEndpoint);
	SetupListen();
}

template<typename T, typename N>
void NetworkHandler<T, N>::SetupListen()
{
	socket->async_receive_from(asio::buffer(buffer),
		remoteEndpoint,
		[&](const asio::error_code& error, size_t bytes)
		{
			if (error) {
				std::cout << "Receive failed: " << error.message() << "\n";
				return;
			}
			std::cout << "Received: '" << std::string(buffer.begin(), buffer.begin() + bytes) << "' (" << error.message() << ")\n";
			running = true;
			SetupListen();
		});
	if (!running)
	{
		context.run();
	}

}


using UdpNetworkHandler = NetworkHandler<asio::ip::udp::socket, asio::ip::udp::endpoint>;
using TcpNetworkHandler = NetworkHandler<asio::ip::tcp::socket, asio::ip::tcp::endpoint>;