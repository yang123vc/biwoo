// IoService.h file here
#ifndef __IoService_h__
#define __IoService_h__

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

///////////////////////////////////////////////
// IoService class
class IoService
{
public:
	typedef boost::shared_ptr<IoService> pointer;

	static IoService::pointer create(void)
	{
		return IoService::pointer(new IoService());
	}

	void start(void)
	{
		//m_ioservice.reset();
		if (m_pProcEventLoop == NULL)
			m_pProcEventLoop = new boost::thread(boost::bind(&IoService::do_event_loop, &m_ioservice));
	}

	void stop(void)
	{
		m_ioservice.stop();
		if (m_pProcEventLoop)
		{
			m_pProcEventLoop->join();
			delete m_pProcEventLoop;
			m_pProcEventLoop = NULL;
		}
	}
	bool is_start(void) const
	{
		return (m_pProcEventLoop != NULL);
	}
	boost::asio::io_service & ioservice(void) {return m_ioservice;}

private:
	static void do_event_loop(boost::asio::io_service * ipservice)
	{
		if (ipservice)
		{
			try
			{
				ipservice->run();
			}catch (std::exception &)
			{
			}
		}
	}

public:
	IoService(void)
		: m_pProcEventLoop(NULL)
	{
	}
	~IoService(void)
	{
		stop();
	}
private:
	boost::asio::io_service m_ioservice;
	boost::thread * m_pProcEventLoop;
};

#endif // __IoService_h__
