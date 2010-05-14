// DSData.h file here
#ifndef __DSData_h__
#define __DSData_h__

//#include "boost/shared_ptr.hpp"

class CDSData
{
public:
	/*typedef boost::shared_ptr<CDSData> pointer;

	static CDSData::pointer create(bool isVideo)
	{
		return CDSData::pointer(new CDSData(isVideo));
	}
	static CDSData::pointer create(struct timeval presentationTime, bool isVideo)
	{
		return CDSData::pointer(new CDSData(presentationTime, isVideo));
	}*/
public:
	struct timeval presentationTime(void) const {return m_presentationTime;}
	bool isVideo(void) const {return m_bIsVideo;}

public:
	CDSData(bool isVideo)
		: m_bIsVideo(isVideo)
	{
		m_presentationTime.tv_sec = 0;
		m_presentationTime.tv_usec = 0;
	}
	CDSData(struct timeval presentationTime, bool isVideo)
		: m_presentationTime(presentationTime)
		, m_bIsVideo(isVideo)
	{
	}
	virtual ~CDSData(void)
	{
	}


private:
	struct timeval m_presentationTime;
	bool m_bIsVideo;
};


#endif // __DSData_h__
