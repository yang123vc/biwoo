// TcpConnection.h file here
#ifndef __TcpConnection_h__
#define __TcpConnection_h__

#include <boost/asio.hpp>
using boost::asio::ip::tcp;
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "../../stl/locklist.h"
#include <iostream>
#include "ReceiveBuffer.h"

////////////////////////
//

class TcpConnection;
typedef boost::shared_ptr<TcpConnection> TcpConnectionPointer;
class TcpConnection_Handler
{
public:
	virtual void OnRemoteRecv(TcpConnectionPointer pRemote, ReceiveBuffer::pointer data)=0;
	virtual void OnRemoteAccept(TcpConnectionPointer pRemote)=0;
	virtual void OnRemoteClose(TcpConnectionPointer pRemote)=0;
};


class TcpConnection
	: public boost::enable_shared_from_this<TcpConnection>
{
private:
	tcp::socket m_socket;
	TcpConnection_Handler * m_handler;
	bool m_killed;
	boost::thread * m_proc_data;
	CLockList<ReceiveBuffer::pointer> m_datas;

public:
	//typedef boost::shared_ptr<TcpConnection> pointer;
	static TcpConnectionPointer create(boost::asio::io_service& io_service, TcpConnection_Handler * handler)
	{
		return TcpConnectionPointer(new TcpConnection(io_service, handler));
	}
	~TcpConnection(void)
	{
		m_killed = true;
		m_proc_data->join();
		delete m_proc_data;

		m_socket.close();
#ifdef _DEBUG
		std::cout << "TcpConnection destructor" << std::endl;
#endif
	}

	tcp::socket& socket() {return m_socket;}

	void start(void)
	{
		if (m_handler)
			m_handler->OnRemoteAccept(shared_from_this());

		start_read();
	}

	void read_some_handler(ReceiveBuffer::pointer buffer, const boost::system::error_code& error, std::size_t size)
	{
		if ( !error )
		{
			buffer->size(size);
			m_datas.add(buffer);
			//if (m_handler)
			//	m_handler->OnRemoteRecv(shared_from_this(), buffer, size);
			start_read();
		}else
		{
			ShowNetWorkError(error);
			// ? add some error_code
			switch (error.value())
			{
			case 10054:
				{
					if (m_handler)
						m_handler->OnRemoteClose(shared_from_this());
					m_socket.close();
				}break;
			default:
				break;
			}
		}
	}

	void proc_Data(void)
	{
		while (!m_killed)
		{
			ReceiveBuffer::pointer buffer;
			if (!m_datas.front(buffer))
			{
#ifdef WIN32
				Sleep(10);
#else
				usleep(10000);
#endif
				continue;
			}

		if (m_handler)
			m_handler->OnRemoteRecv(shared_from_this(), buffer);
		}
	}

private:
	void start_read(void)
	{
		ReceiveBuffer::pointer newBuffer = ReceiveBuffer::createNew();
		m_socket.async_read_some(boost::asio::buffer(const_cast<unsigned char*>(newBuffer->data()), Max_ReceiveBuffer_ReceiveSize),
			boost::bind(&TcpConnection::read_some_handler, shared_from_this(), newBuffer,
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
			);
	}

	TcpConnection(boost::asio::io_service& io_service, TcpConnection_Handler * handler)
		: m_socket(io_service)
		, m_handler(handler)
		, m_killed(false), m_proc_data(0)
	{
		m_proc_data = new boost::thread(boost::bind(&TcpConnection::do_proc_data, this));
	}

	//void handle_write(const boost::system::error_code& /*error*/,
	//	size_t /*bytes_transferred*/)
	//{
	//}

	void ShowNetWorkError( const boost::system::error_code &error )
	{
		std::cout << error.message() << std::endl;
	}

	static void do_proc_data(TcpConnection * owner)
	{
		BOOST_ASSERT (owner != 0);

		owner->proc_Data();
	}
};


#endif // __TcpConnection_h__
