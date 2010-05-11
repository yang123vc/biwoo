// UdpSocket.h file here
#ifndef __UdpSocket_h__
#define __UdpSocket_h__

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include "../../stl/locklist.h"
using boost::asio::ip::udp;
#include "UdpEndPoint.h"

//////////////////////////////////////////////
// UdpSocket_Handler
class UdpSocket;
class UdpSocket_Handler
{
public:
	virtual void OnReceiveData(const UdpSocket & UdpSocket, UdpEndPoint::pointer endpoint) = 0;

};

///////////////////////////////////////////////
// UdpSocket class
class UdpSocket
{
public:
	typedef boost::shared_ptr<UdpSocket> pointer;
	static UdpSocket::pointer create(UdpSocket_Handler * handler)
	{
		return UdpSocket::pointer(new UdpSocket(handler));
	}

	// udpPort == 0; ¶¯Ì¬
	void start(boost::asio::io_service & ioservice, unsigned short udpPort=0)
	{
		if (m_socket == NULL)
			m_socket = new udp::socket(ioservice, udp::endpoint(udp::v4(), udpPort));

		if (m_proc_data == 0)
			m_proc_data = new boost::thread(boost::bind(&UdpSocket::do_proc_data, this));

		start_receive();
	}
	void stop(void)
	{
		udp::socket * pSocketTemp = m_socket;
		m_socket = NULL;
		if (pSocketTemp)
			delete pSocketTemp;

		if (m_proc_data)
		{
			m_proc_data->join();
			delete m_proc_data;
			m_proc_data = 0;
		}

		m_endpoints.clear();
	}
	bool is_start(void) const {return m_socket != NULL;}
	udp::socket * socket(void) {return m_socket;}
	void write(const unsigned char * data, size_t size, const udp::endpoint & endpoint)
	{
		boost::system::error_code ignored_error;
		if (m_socket)
		{
			m_socket->send_to(boost::asio::buffer(data, size),
				endpoint, 0, ignored_error);
		}
	}

	void proc_Data(void)
	{
		while (m_socket != 0)
		{
			UdpEndPoint::pointer endpoint;
			if (!m_endpoints.front(endpoint))
			{
#ifdef WIN32
				Sleep(10);
#else
				usleep(10000);
#endif
				continue;
			}

		if (m_handler)
			m_handler->OnReceiveData(*this, endpoint);
		}
	}

private:
	void start_receive(void)
	{
		if (m_socket)
		{
			UdpEndPoint::pointer new_endpoint = UdpEndPoint::create();

			m_socket->async_receive_from(boost::asio::buffer(const_cast<unsigned char*>(new_endpoint->buffer()), Max_UdpSocket_ReceiveSize),
				new_endpoint->endpoint(),
				boost::bind(&UdpSocket::receive_handler, this, new_endpoint,
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
	}

	void receive_handler(UdpEndPoint::pointer endpoint, const boost::system::error_code& error, std::size_t size)
	{
		endpoint->size(size);
		m_endpoints.add(endpoint);
		//if (m_handler)
		//	m_handler->OnReceiveData(*this, endpoint, size);
		start_receive();
	}

	static void do_proc_data(UdpSocket * owner)
	{
		BOOST_ASSERT (owner != 0);

		owner->proc_Data();
	}
public:
	UdpSocket(UdpSocket_Handler * handler)
		: m_handler(handler)
		, m_socket(NULL)
		, m_proc_data(0)
	{
	}
	~UdpSocket(void)
	{
		stop();
	}
private:
	udp::socket * m_socket;
	UdpSocket_Handler * m_handler;
	udp::endpoint m_endpointlocal;
	boost::thread * m_proc_data;
	CLockList<UdpEndPoint::pointer> m_endpoints;
};

#endif // __UdpSocket_h__
