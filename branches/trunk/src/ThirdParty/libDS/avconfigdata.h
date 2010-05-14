// AVConfigData.h file here
#ifndef __AVConfigData_h__
#define __AVConfigData_h__

#include "boost/shared_ptr.hpp"
#include "AVParameter.h"

#define MST_RGB32 1
#define MST_RGB24  2
#define MST_YUY2  3
#define MST_I420  4
#define MST_MJPG  5
#define MST_H264  6

class CAVConfigData
{
public:
	

	typedef boost::shared_ptr<CAVConfigData> pointer;

	static CAVConfigData::pointer create(void)
	{
		return CAVConfigData::pointer(new CAVConfigData());
	}

	CAVConfigData(void)
		: m_configStr(NULL)
		, m_configSize(0)
		, m_numChannels(2)
		, m_frequency(0)
		, m_duration(0.0)
		, m_isVideo(false)
		, m_MediaSubtype(MST_RGB32)
		, m_width(320), m_height(240)
	{
	}
	~CAVConfigData(void)
	{
		if (m_configStr)
			delete[] m_configStr;
	}

	////////////////////////////////////////////////////////
	// Method:
	/*CConfigData * copyNew(void) const
	{
		CConfigData * result = new CConfigData();
		result->setConfigData(this->getConfigStr(), this->getConfigSize());
		result->setNumChannels(this->getNumChannels());
		result->setFrequency(this->getFrequency());
		result->setDuration(this->getDuration());
		result->setIsVideo(this->getIsVideo());
		return result;
	}*/


	void setConfigData(const unsigned char * configStr, unsigned int configSize)
	{
		if (m_configStr)
		{
			delete[] m_configStr;
			m_configStr = NULL;
		}

		if (configStr == NULL || configSize <= 0) return;

		m_configSize = configSize;
		m_configStr = new unsigned char[m_configSize];
		memcpy(m_configStr, configStr, m_configSize);
	}
	const unsigned char * getConfigStr(void) const {return m_configStr;}
	unsigned int getConfigSize(void) const {return m_configSize;}

	void setNumChannels(unsigned int numChannels) {m_numChannels = numChannels;}
	unsigned int getNumChannels(void) const {return m_numChannels;}

	void setFrequency(unsigned int frequency) {m_frequency = frequency;}
	unsigned int getFrequency(void) const {return m_frequency;}

	void setDuration(float duration) {m_duration = duration;}
	float getDuration(void) const {return m_duration;}

	void setIsVideo(bool isVideo) {m_isVideo = isVideo;}
	bool getIsVideo(void) const {return m_isVideo;}

	void setWidth(int newv) {m_width = newv;}
	int getWidth(void) const {return m_width;}

	void setHeight(int newv) {m_height = newv;}
	int getHeight(void) const {return m_height;}

	void setMediaSubtype(int subtype) {m_MediaSubtype = subtype;}
	int getMediaSubtype(void) const {return m_MediaSubtype;}

	void timestampType(CAVParameter::TimestampType newv) {m_timestampType = newv;}
	CAVParameter::TimestampType timestampType(void) const {return m_timestampType;}

private:
	unsigned char * m_configStr;
	unsigned int m_configSize;
	unsigned int m_numChannels;
	unsigned int m_frequency;
	float m_duration;
	bool m_isVideo;
	int m_width;
	int m_height;
    
	int m_MediaSubtype;
	CAVParameter::TimestampType m_timestampType;
};


#endif // __AVConfigData_h__
