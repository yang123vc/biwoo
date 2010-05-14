// AVData.h file here
#ifndef __AVData_h__
#define __AVData_h__

#include "boost/shared_ptr.hpp"

class CAVData
{
public:
	typedef boost::shared_ptr<CAVData> pointer;

	static CAVData::pointer create(unsigned char * data, long size, unsigned int timestamp, bool isVideo)
	{
		return CAVData::pointer(new CAVData(data, size, timestamp, isVideo));
	}
	static CAVData::pointer create(const unsigned char * data, long size, unsigned int timestamp, bool isVideo)
	{
		return CAVData::pointer(new CAVData(data, size, timestamp, isVideo));
	}

public:
	const unsigned char * data(void) const {return m_data;}
	long size(void) const {return m_size;}
	bool isVideo(void) const {return m_bIsVideo;}

	void timestamp(unsigned int newv) {m_timestamp = newv;}
	unsigned int timestamp(void) const {return m_timestamp;}

public:
	CAVData(unsigned char * data, long size, unsigned int timestamp, bool isVideo)
		: m_size(size)
		, m_bIsVideo(isVideo)
		, m_timestamp(timestamp)
	{
		m_data = data;
	}
	CAVData(const unsigned char * data, long size, unsigned int timestamp, bool isVideo)
		: m_size(size)
		, m_bIsVideo(isVideo)
		, m_timestamp(timestamp)
	{
		if (data != 0 && m_size > 0)
		{
			m_data = new unsigned char[m_size];
			memcpy(m_data, data, m_size);
		}else
		{
			m_data = 0;
			m_size = 0;
		}

	}
	virtual ~CAVData(void)
	{
		if (m_data != 0)
			delete[] m_data;
	}

private:
	unsigned char * m_data;
	long m_size;
	//struct timeval m_presentationTime;
	bool m_bIsVideo;
	unsigned int m_timestamp;
};


#endif // __AVData_h__
