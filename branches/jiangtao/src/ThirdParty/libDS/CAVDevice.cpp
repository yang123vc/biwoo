//
// CAVDevice.cpp
//

/*-----------------------------------------------------*\
			HQ Tech, Make Technology Easy!       
 More information, please go to http://hqtech.nease.net.
/*-----------------------------------------------------*/

#include "CAVDevice.h"

/////////////////////////////////////////////////////////////////////////
CAVDevice::CAVDevice()
{
	mType = DT_Unknown;
	mFriendlyName    = "";
	mDisplayName     = "";
	mVideoResolution = 0;
	mAudioConnector  = 0;
}

CAVDevice::~CAVDevice()
{
}

CAVDevice& CAVDevice::operator = (const CAVDevice& inDevice)
{
	mType         = inDevice.mType;
	mFriendlyName = inDevice.mFriendlyName;
	mDisplayName  = inDevice.mDisplayName;
	mVideoResolution = inDevice.mVideoResolution;
	mAudioConnector  = inDevice.mAudioConnector;
	return *this;
}

void CAVDevice::SetDevideType(Device_Type inType)
{
	mType = inType;
}

Device_Type CAVDevice::GetDeviceType(void)
{
	return mType;
}

bool CAVDevice::IsDVDevice(void)
{
	return (mType == DT_DV);
}

bool CAVDevice::IsDeviceAvailable(void)
{
	return (mType != DT_Unknown && mDisplayName != "");
}

void CAVDevice::SetDeviceFriendlyName(const char * inName)
{
	mFriendlyName = inName;
}

std::string CAVDevice::GetDeviceFriendlyName(void)
{
	return mFriendlyName;
}

void CAVDevice::SetDeviceDisplayName(const char * inName)
{
	mDisplayName = inName;
}

std::string CAVDevice::GetDeviceDisplayName(void)
{
	return mDisplayName;
}

void CAVDevice::SetVideoResolution(long inResolution)
{
	mVideoResolution = inResolution;
}

long CAVDevice::GetVideoResolution(void)
{
	return mVideoResolution;
}

void CAVDevice::SetAudioConnector(long inConnector)
{
	mAudioConnector = inConnector;
}

long CAVDevice::GetAudioConnector(void)
{
	return mAudioConnector;
}