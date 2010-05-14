// UdpEndPoint.h file here
#ifndef __UdpEndPoint_h__
#define __UdpEndPoint_h__

#include <iostream>
#include <boost/asio.hpp>
using boost::asio::ip::udp;
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

const size_t Max_UdpSocket_ReceiveSize	= 5*1024;
////////////////////////
// UdpEndPoint class
class UdpEndPoint
	: public boost::enable_shared_from_this<UdpEndPoint>
{
public:
	typedef boost::shared_ptr<UdpEndPoint> pointer;
	static pointer create(void)
	{
		return pointer(new UdpEndPoint());
	}

	~UdpEndPoint()
	{
#ifdef _DEBUG
		std::cout << "UdpEndPoint destructor" << std::endl;
#endif
	}

	udp::endpoint& endpoint(void) {return remote_endpoint;}
	const unsigned char * buffer(void) const {return m_buffer;}
	unsigned long getId(void) const
	{
		boost::asio::ip::address address = remote_endpoint.address();
		if (address.is_v4())
		{
			return address.to_v4().to_ulong() + remote_endpoint.port();	// ? +port	
		}else
		{
			// ??? IPV6
			return address.to_v6().scope_id() + remote_endpoint.port();	
		}
	}

	void size(int newv) {m_size = newv;}
	int size(void) const {return m_size;}

private:
	UdpEndPoint(void)
		: m_size(0)
	{
		memset(m_buffer, 0, Max_UdpSocket_ReceiveSize);
	}

private:
	udp::endpoint remote_endpoint;
	unsigned char m_buffer[Max_UdpSocket_ReceiveSize];
	int m_size;
};

#endif // __UdpEndPoint_h__
