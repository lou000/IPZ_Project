#pragma once
#include <thread>
#include<iostream>
#include <algorithm>
#include <memory>
#include <thread>
#include <mutex>
#include <deque>
#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>


enum class MsgDestination {
	Multicast,
	Client,
	Server
};
enum class MsgType {
	Connect,
	Disconnect,
	Update,
	Ready
};

struct Header
{
	MsgType type;
	MsgDestination dest;
	int dest_id;
	int src_id;
	int size;
};

struct Message
{

	// Header & Body vector
	Header header;
	std::vector<uint8_t> body;

	// returns size of entire message packet in bytes
	size_t size() const
	{
		return body.size();
	}

	friend std::ostream& operator << (std::ostream& os, const Message& msg)
	{
		os << "dest ID:" << int(msg.header.dest_id) << "src ID:" << int(msg.header.dest_id) << "src ID:"  << " Size:" << msg.header.size;
		return os;
	}


	// Pushes any POD-like data into the message buffer
	template<typename DataType>
	friend Message& operator << (Message& msg, const DataType& data)
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
	friend Message& operator >> (Message& msg, DataType& data)
	{
		// Check that the type of the data being pushed is trivially copyable
		static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pulled from vector");

		// Cache the location towards the end of the vector where the pulled data starts
		size_t i = msg.body.size() - sizeof(DataType);

		// Physically copy the data from the vector into the user variable
		std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

		//// Shrink the vector to remove read bytes, and reset end position
		//msg.body.resize(i);

		//// Recalculate the message size
		//msg.header.size = msg.size();

		//// Return the target message so it can be "chained"
		return msg;
	}
};

enum class Owner
{
	Server,
	Client
};

template<typename T>
class tsqueue
{
public:
	tsqueue() = default;
	tsqueue(const tsqueue<T>&) = delete;
	virtual ~tsqueue() { clear(); }

public:
	// Returns and maintains item at front of Queue
	const T& front()
	{
		std::scoped_lock lock(muxQueue);
		return deqQueue.front();
	}

	// Returns and maintains item at back of Queue
	const T& back()
	{
		std::scoped_lock lock(muxQueue);
		return deqQueue.back();
	}

	// Removes and returns item from front of Queue
	T pop_front()
	{
		std::scoped_lock lock(muxQueue);
		auto t = std::move(deqQueue.front());
		deqQueue.pop_front();
		return t;
	}

	// Removes and returns item from back of Queue
	T pop_back()
	{
		std::scoped_lock lock(muxQueue);
		auto t = std::move(deqQueue.back());
		deqQueue.pop_back();
		return t;
	}

	// Adds an item to back of Queue
	void push_back(const T& item)
	{
		std::scoped_lock lock(muxQueue);
		deqQueue.emplace_back(std::move(item));

		std::unique_lock<std::mutex> ul(muxBlocking);
		cvBlocking.notify_one();
	}

	// Adds an item to front of Queue
	void push_front(const T& item)
	{
		std::scoped_lock lock(muxQueue);
		deqQueue.emplace_front(std::move(item));

		std::unique_lock<std::mutex> ul(muxBlocking);
		cvBlocking.notify_one();
	}

	// Returns true if Queue has no items
	bool empty()
	{
		std::scoped_lock lock(muxQueue);
		return deqQueue.empty();
	}

	// Returns number of items in Queue
	size_t count()
	{
		std::scoped_lock lock(muxQueue);
		return deqQueue.size();
	}

	// Clears Queue
	void clear()
	{
		std::scoped_lock lock(muxQueue);
		deqQueue.clear();
	}

	void wait()
	{
		while (empty())
		{
			std::unique_lock<std::mutex> ul(muxBlocking);
			cvBlocking.wait(ul);
		}
	}

protected:
	std::mutex muxQueue;
	std::deque<T> deqQueue;
	std::condition_variable cvBlocking;
	std::mutex muxBlocking;
};


struct UDPConnection
{
	UDPConnection(asio::ip::udp::endpoint elem) : elem(std::move(elem))//, context(asioContext)
	{
		id = counter++;
	}

	asio::ip::udp::endpoint elem;
	int id;
	inline static int counter = 0;
};
struct TCPConnection
{
	TCPConnection(asio::ip::tcp::socket elem) : elem(std::move(elem))//, context(asioContext)
	{
		id = counter++;
	}
	asio::ip::tcp::socket elem;
	int id;
	inline static int counter = 0;
};


template<typename Protocol,typename Conn ,typename U = asio::ip::tcp::acceptor>
class NetworkHandler
{
public:

	NetworkHandler(Owner owner) : owner(owner), idleWork(context), socket(context) {}
	void Connect(std::string address, unsigned short port);
	void Disconnect();

	void Send(Message msg)
	{
		sendQueue.push_back(msg);
		SendHeader(msg);
	}

	void RegisterCallback(std::function<void()> callback,MsgType type)
	{
		switch (type)
		{
		case MsgType::Connect:
			connectionCallback = callback;
			break;
		case MsgType::Disconnect:
			disconnectionCallback = callback;
			break;
		case MsgType::Update:
			updateCallback = callback;
			break;
		default:
			break;
		}
	}
	void Listen();
	void StartContext();
	typename Protocol::endpoint GetEndPoint() const { return remoteEndpoint; }
	void RemoveConnectionAt(int id){ connections.erase(connections.begin()+id); }
	void ReadHeader();
	int getMostRecentConnectionId() const { return connections.size() -1; }
	void ReadBody();
	void AddEndpoint(typename Protocol::endpoint endpoint) { connections.emplace_back(Conn(endpoint)); }
	Message GetLastMessage() const { return tempMsg; }
private:

	void Notify()
	{
		switch (tempMsg.header.type)
		{
		case MsgType::Connect:
			connectionCallback();
			break;
		case MsgType::Disconnect:
			disconnectionCallback();
			break;
		case MsgType::Update:
			updateCallback();
			break;
		}

	}
	void SendHeader(Message msg);
	void SendBody(Message msg);
	typename Protocol::endpoint remoteEndpoint;
	asio::io_context context;
	asio::io_context::work idleWork;
	Owner owner;
	std::thread contextThread;
	std::unique_ptr<U> acceptor;
	typename Protocol::socket socket;
	std::function<void()> connectionCallback;
	std::function<void()> disconnectionCallback;
	std::function<void()> updateCallback;
	std::vector<Conn> connections;
	Message tempMsg;
	tsqueue<Message> sendQueue;
	void ReadBodyHandler(asio::error_code ec, std::size_t bytes_transferred)
	{
		if (ec)
		{
			std::cout << "Receive failed: " << ec.message() << "\n";
		}
		else
		{
			std::cout << "Received body: '" << tempMsg << "' (" << ec.message() << ")\n";
			Notify();
		}
	}
	void ReadHeaderHandler(asio::error_code ec, std::size_t bytes_transferred)
	{
		if (ec)
		{
			std::cout << "Receive failed: " << ec.message() << "\n";
		}
		else
		{
			std::cout << "Received header: '" << tempMsg << "' (" << ec.message() << ")\n";
			if (tempMsg.header.size > 0)
			{
				ReadBody();
			}
			else
			{
				Notify();
			}
		}
		ReadHeader();
	}

	void WriteBodyHandler(asio::error_code ec, std::size_t bytes_transferred)
	{
		std::cout << "Wrote body " << bytes_transferred << " bytes with " <<
			ec.message() << std::endl;
		sendQueue.pop_front();
		if (!sendQueue.empty())
		{
			SendHeader(sendQueue.front());
		}
	}
	void WriteHeaderHandler(asio::error_code ec, std::size_t bytes_transferred)
	{
		std::cout << "wrote " << bytes_transferred << " bytes with " <<
			ec.message() << std::endl;
		if (sendQueue.front().header.size > 0)
			SendBody(sendQueue.front());
		else
			sendQueue.pop_front();
	}
};

template<>
inline void NetworkHandler<asio::ip::tcp, TCPConnection>::Connect(std::string address, unsigned short port)
{
	if (owner == Owner::Server)
	{
		acceptor = std::make_unique<asio::ip::tcp::acceptor>(asio::ip::tcp::acceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)));
		socket.open(asio::ip::tcp::v4());
	}
		
	if (owner == Owner::Client)
	{
		asio::ip::tcp::resolver resolver(context);
		asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(address, std::to_string(port));
		asio::async_connect(socket, endpoints,
			[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
			{
				if (!ec)
				{
					std::cout << "Connected with tcp \n";
					connectionCallback();
				}
				else
				{
					ec.message();
				}
			});
	}

	StartContext();
}
template<typename Protocol, typename Conn, typename U>
inline void NetworkHandler<Protocol,Conn, U>::Disconnect()
{
	context.stop();
	if (contextThread.joinable())
		contextThread.join();
}
template<>
inline void NetworkHandler<asio::ip::udp, UDPConnection>::Connect(std::string address, unsigned short port)
{
	remoteEndpoint.address(asio::ip::address::from_string(address));
	remoteEndpoint.port(port);
	socket.open(asio::ip::udp::v4());
	StartContext();
}

template<>
inline void NetworkHandler<asio::ip::udp, UDPConnection>::Listen()
{
	socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), remoteEndpoint.port()));
	ReadHeader();
}

template<> 
inline void NetworkHandler<asio::ip::tcp, TCPConnection>::Listen()
{
	acceptor->async_accept(
		[&](std::error_code error, asio::ip::tcp::socket soc)
		{
			if (error)
			{
				std::cout << "Connection Error: " << error.message() << '\n';
			}
			else
			{
				std::cout << "New Connection: " << soc.remote_endpoint() << "\n";
				remoteEndpoint = soc.remote_endpoint();
				connections.emplace_back(TCPConnection(std::move(soc)));
				connectionCallback();
			}
			Listen();
		}
	);

}

template<typename T,typename Conn, typename U>
inline void NetworkHandler<T,Conn, U>::StartContext()
{
	contextThread = std::thread([this] { context.run();});
}

template<>
inline void NetworkHandler<asio::ip::udp, UDPConnection>::ReadHeader()
{
	if (owner == Owner::Server)
		for (auto& conn : connections)
			socket.async_receive_from(asio::buffer(&tempMsg.header, sizeof(Header)),
			conn.elem,
			[this](const asio::error_code& error, size_t bytes)
			{
				ReadHeaderHandler(error, bytes);
			});
	else
		socket.async_receive_from(asio::buffer(&tempMsg.header, sizeof(Header)),
		remoteEndpoint,
		[this](const asio::error_code& error, size_t bytes)
		{
			ReadHeaderHandler(error, bytes);
		});
}


template<>
inline void NetworkHandler<asio::ip::tcp, TCPConnection>::ReadHeader()
{
	if(owner == Owner::Server)
		for (auto& conn : connections)
		{
			asio::async_read(conn.elem, asio::buffer(&tempMsg.header,sizeof(Header)), asio::transfer_at_least(sizeof(Header)),
				[this](std::error_code error, std::size_t bytes)
				{
					ReadHeaderHandler(error, bytes);
				});
		}
	else
	{
		asio::async_read(socket, asio::buffer(&tempMsg.header, sizeof(Header)), asio::transfer_at_least(sizeof(Header)),
			[this](std::error_code error, std::size_t bytes)
			{
				ReadHeaderHandler(error, bytes);
			});
	}
}

template<>
inline void NetworkHandler<asio::ip::tcp, TCPConnection>::ReadBody()
{
	tempMsg.body.resize(tempMsg.header.size);
	if (owner == Owner::Server)
		for (auto& conn : connections)
		{
			asio::async_read(conn.elem, asio::buffer(tempMsg.body.data(), tempMsg.body.size()), asio::transfer_at_least(tempMsg.body.size()),
				[this](std::error_code error, std::size_t bytes)
				{
					ReadBodyHandler(error, bytes);
				});
		}
	else
	{
		asio::async_read(socket, asio::buffer(tempMsg.body.data(), tempMsg.body.size()), asio::transfer_at_least(tempMsg.header.size),
			[this](std::error_code error, std::size_t bytes)
			{
				ReadBodyHandler(error, bytes);
				//if (error)
				//{
				//	std::cout << "Receive failed: " << error.message() << "\n";
				//}
				//else
				//{
				//	std::cout << "Received body: '" << tempMsg << "' (" << error.message() << ")\n";
				//	Notify();
				//}
			});
	}
}

template<>
inline void NetworkHandler<asio::ip::udp, UDPConnection>::ReadBody()
{
	tempMsg.body.resize(tempMsg.header.size);
	if (owner == Owner::Server)
		for (auto& conn : connections)
			socket.async_receive_from(asio::buffer(tempMsg.body.data(), tempMsg.body.size()),
			conn.elem,
			[this](const asio::error_code& error, size_t bytes)
			{
				ReadBodyHandler(error, bytes);
			});
	else
		socket.async_receive_from(asio::buffer(tempMsg.body.data(), tempMsg.body.size()),
		remoteEndpoint,
		[this](const asio::error_code& error, size_t bytes)
		{
			ReadBodyHandler(error, bytes);
		});
}

template<>
inline void NetworkHandler<asio::ip::tcp, TCPConnection>::SendHeader(Message msg)
{
	if (owner == Owner::Server)
	{
		if (msg.header.dest == MsgDestination::Multicast)
			for (auto& conn : connections)
			{
				asio::async_write(conn.elem, asio::buffer(&msg.header, sizeof(Header)),
					[this]
					(const asio::error_code& error, std::size_t bytes_transferred)
					{
						WriteHeaderHandler(error, bytes_transferred);
					});
			}
		if (msg.header.dest == MsgDestination::Client)
		{
			asio::async_write(connections.at(msg.header.dest_id).elem, asio::buffer(&msg.header, sizeof(Header)),
				[this]
				(const asio::error_code& error, std::size_t bytes_transferred)
				{
					WriteHeaderHandler(error, bytes_transferred);
				});
		}
	}
	else
	{
		asio::async_write(socket, asio::buffer(&msg.header, sizeof(Header)),
			[this]
			(const asio::error_code& error, std::size_t bytes_transferred)
			{
				WriteHeaderHandler(error, bytes_transferred);
			});
	}
}
template<>
inline void NetworkHandler<asio::ip::udp, UDPConnection>::SendHeader(Message msg)
{
	if (owner == Owner::Server)
	{
		if (msg.header.dest == MsgDestination::Multicast)
			for (auto& conn : connections)
			{
				socket.async_send_to(asio::buffer(&msg.header, sizeof(Header)), conn.elem,
				[this]
				(const asio::error_code& error, std::size_t bytes_transferred)
				{
						WriteHeaderHandler(error, bytes_transferred);
				});
			}
		if (msg.header.dest == MsgDestination::Client)
		{
			socket.async_send_to(asio::buffer(&msg.header, sizeof(Header)), connections.at(msg.header.dest_id).elem,
			[this]
			(const asio::error_code& error, std::size_t bytes_transferred)
			{
				WriteHeaderHandler(error, bytes_transferred);
			});
		}
	}
	else
	{
		socket.async_send_to(asio::buffer(&msg.header, sizeof(Header)), remoteEndpoint,
		[this]
		(const asio::error_code& error, std::size_t bytes_transferred)
		{
			WriteHeaderHandler(error, bytes_transferred);
		});
	}
}

template<>
inline void NetworkHandler<asio::ip::tcp, TCPConnection>::SendBody(Message msg)
{
	if (owner == Owner::Server)
	{
		if (msg.header.dest == MsgDestination::Multicast)
			for (auto& conn : connections)
			{
				asio::async_write(conn.elem, asio::buffer(msg.body.data(), msg.body.size()),
				[this]
				(const asio::error_code& error, std::size_t bytes_transferred)
				{
					WriteBodyHandler(error, bytes_transferred);
				});
			}
		if (msg.header.dest == MsgDestination::Client)
		{
			asio::async_write(connections.at(msg.header.dest_id).elem, asio::buffer(msg.body.data(), msg.body.size()),
			[this]
			(const asio::error_code& error, std::size_t bytes_transferred)
			{
				WriteBodyHandler(error, bytes_transferred);
			});
		}
	}
	else
	{
		asio::async_write(socket, asio::buffer(msg.body.data(), msg.body.size()),
		[this]
		(const asio::error_code& error, std::size_t bytes_transferred)
		{
			WriteBodyHandler(error, bytes_transferred);
		});
	}
}

template<>
inline void NetworkHandler<asio::ip::udp, UDPConnection>::SendBody(Message msg)
{
	if (owner == Owner::Server)
	{
		if (msg.header.dest == MsgDestination::Multicast)
			for (auto& conn : connections)
			{
				socket.async_send_to(asio::buffer(msg.body.data(), msg.body.size()), conn.elem,
				[this]
				(const asio::error_code& error, std::size_t bytes_transferred)
				{
						WriteBodyHandler(error, bytes_transferred);
				});
			}
		if (msg.header.dest == MsgDestination::Client)
		{
			socket.async_send_to(asio::buffer(msg.body.data(), msg.body.size()), connections.at(msg.header.dest_id).elem,
			[this]
			(const asio::error_code& error, std::size_t bytes_transferred)
			{
				WriteBodyHandler(error, bytes_transferred);
			});
		}
	}
	else
	{
		socket.async_send_to(asio::buffer(msg.body.data(), msg.body.size()), remoteEndpoint,
		[this]
		(const asio::error_code& error,std::size_t bytes_transferred)
		{
			WriteBodyHandler(error, bytes_transferred);
		});
	}
}

using UdpNetworkHandler = NetworkHandler<asio::ip::udp,UDPConnection>;
using TcpNetworkHandler = NetworkHandler<asio::ip::tcp, TCPConnection>;