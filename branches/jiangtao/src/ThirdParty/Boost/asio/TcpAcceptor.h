// TcpAcceptor.h file here
#ifndef __TcpAcceptor_h__
#define __TcpAcceptor_h__

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
using boost::asio::ip::tcp;
#include "TcpConnection.h"

///////////////////////////////////////////////
// TcpAcceptor class
class TcpAcceptor
{
private:
	tcp::acceptor * m_acceptor;
	TcpConnection_Handler * m_handler;

public:
	void start(boost::asio::io_service & ioservice, size_t tcpPort)
	{
		if (m_acceptor == NULL)
			m_acceptor = new tcp::acceptor(ioservice, tcp::endpoint(tcp::v4(), tcpPort));
		m_acceptor->set_option(tcp::acceptor::reuse_address(true));
		start_accept();
	}
	void stop(void)
	{
		if (m_acceptor)
		{
			m_acceptor->close();
			delete m_acceptor;
			m_acceptor = NULL;
		}
	}
private:
	void start_accept(void)
	{
		if (m_acceptor != NULL) 
		{
			TcpConnectionPointer new_connection =	TcpConnection::create(m_acceptor->io_service(), m_handler);

			m_acceptor->async_accept(new_connection->socket(),
				boost::bind(&TcpAcceptor::handle_accept, this, new_connection,
				boost::asio::placeholders::error));
		}
	}

	void handle_accept(TcpConnectionPointer new_connection,
		const boost::system::error_code& error)
	{
		if (!error)
		{
			new_connection->start();
			start_accept();	//准备下一个client
		}
	}

public:
	TcpAcceptor(TcpConnection_Handler * handler)
		: m_acceptor(NULL)
		, m_handler(handler)
	{
	}
	~TcpAcceptor(void)
	{
		stop();
	}

};

#endif // __TcpAcceptor_h__
