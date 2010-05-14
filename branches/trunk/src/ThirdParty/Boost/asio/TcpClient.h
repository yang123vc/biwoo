// TcpClient.h file here
#ifndef __TcpClient_h__
#define __TcpClient_h__

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "../../stl/locklist.h"
using boost::asio::ip::tcp;
#include "ReceiveBuffer.h"

//////////////////////////////////////////////
// TcpClient_Handler
class TcpClient;
typedef boost::shared_ptr<TcpClient> TcpClientPointer;
class TcpClient_Handler
{
public:
	virtual void OnConnected(TcpClientPointer tcpClient) = 0;
	virtual void OnConnectError(TcpClientPointer tcpClient, const boost::system::error_code & error) = 0;
	virtual void OnReceiveData(TcpClientPointer tcpClient, ReceiveBuffer::pointer data) = 0;
	//virtual void OnDisconnect(const TcpClient & tcpClient);

};

///////////////////////////////////////////////
// TcpClient class
class TcpClient
	: public boost::enable_shared_from_this<TcpClient>
{
public:
	typedef boost::shared_ptr<TcpClient> pointer;
	static TcpClient::pointer create(TcpClient_Handler * handler)
	{
		return TcpClient::pointer(new TcpClient(handler));
	}

	void connect(boost::asio::io_service& io_service, tcp::endpoint& endpoint)
	{
		if (m_socket == NULL)
			m_socket = new tcp::socket(io_service);

		if (m_proc_data == 0)
			m_proc_data = new boost::thread(boost::bind(&TcpClient::do_proc_data, this));

		m_socket->async_connect(endpoint,
		//m_socket.async_connect(endpoint,
			boost::bind(&TcpClient::connect_handler, this,
			boost::asio::placeholders::error)
			);
	}
	void disconnect(void)
	{
		if (m_socket)
		{
			tcp::socket * socktetemp = m_socket;
			m_socket = NULL;
			socktetemp->close();
			delete socktetemp;
		}

		if (m_proc_data)
		{
			m_proc_data->join();
			delete m_proc_data;
			m_proc_data = 0;
		}

		m_datas.clear();
		//m_socket.close();
	}
	bool is_open(void) const
	{
		return m_socket != NULL && m_socket->is_open();
		//return m_socket.is_open();
	}
	void write(const unsigned char * data, size_t size)
	{
		if (m_socket == 0) return;
		boost::asio::write(*m_socket, boost::asio::buffer(data, size));
	}

	//const tcp::socket & socket(void) const {return m_socket;}
	tcp::socket * socket(void) const {return m_socket;}

	void proc_Data(void)
	{
		while (m_socket != 0)
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
			//m_handler->OnReceiveData(*this, buffer);
			m_handler->OnReceiveData(shared_from_this(), buffer);
		}
	}

private:
	void async_read_some(void)
	{
		if (m_socket == 0) return;

		ReceiveBuffer::pointer newBuffer = ReceiveBuffer::createNew();
		m_socket->async_read_some(boost::asio::buffer(const_cast<unsigned char*>(newBuffer->data()), Max_ReceiveBuffer_ReceiveSize),
			boost::bind(&TcpClient::read_some_handler,this, newBuffer,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
			);
	}

	void connect_handler(const boost::system::error_code& error)
	{
		if (m_socket == 0) return;

		if(!error)
		{
			if (m_handler)
				//m_handler->OnConnected(*this);
				m_handler->OnConnected(shared_from_this());

			async_read_some();
		}else
		{
			m_socket->close();
			if (m_handler)
				//m_handler->OnConnectError(*this, error);
				m_handler->OnConnectError(shared_from_this(), error);
		}
	}
	void read_some_handler(ReceiveBuffer::pointer newBuffer, const boost::system::error_code& error, std::size_t size)
	{
		if (m_socket == 0) return;
		if(!error)
		{
			newBuffer->size(size);
			m_datas.add(newBuffer);
			//if (m_handler)
			//	m_handler->OnReceiveData(*this, newBuffer);

			async_read_some();
		}else
		{
			// ??
			m_socket->close();
		}
	}
	//void handle_write(const boost::system::error_code& error)
	//{
	//}

	static void do_proc_data(TcpClient * owner)
	{
		BOOST_ASSERT (owner != 0);

		owner->proc_Data();
	}

public:
	TcpClient(TcpClient_Handler * handler)
		: m_socket(0)
		, m_handler(handler)
		, m_proc_data(0)
	{
	}
	~TcpClient(void)
	{
		disconnect();
	}
private:
    tcp::socket * m_socket;
	TcpClient_Handler * m_handler;
	boost::thread * m_proc_data;
	CLockList<ReceiveBuffer::pointer> m_datas;
};

#endif // __TcpClient_h__
