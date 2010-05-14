//
// CAVDevice.h
//

#ifndef __H_CAVDevice__
#define __H_CAVDevice__

#include "GlobalDefs.h"
#include <string>

class CAVDevice
{
private:
	Device_Type		mType;
	std::string		mFriendlyName;
	std::string		mDisplayName;

	long			mVideoResolution;
	long			mAudioConnector;

public:
	CAVDevice();
	~CAVDevice();

	CAVDevice& operator = (const CAVDevice&);

	void SetDevideType(Device_Type inType);
	Device_Type GetDeviceType(void);

	bool IsDVDevice(void);
	bool IsDeviceAvailable(void);

	void SetDeviceFriendlyName(const char * inName);
	std::string GetDeviceFriendlyName(void);

	void SetDeviceDisplayName(const char * inName);
	std::string GetDeviceDisplayName(void);

	void SetVideoResolution(long inResolution);
	long GetVideoResolution(void);

	void SetAudioConnector(long inConnector);
	long GetAudioConnector(void);
};

#endif // __H_CAVDevice__