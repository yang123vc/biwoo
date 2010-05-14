//
// UDsUtils.h
//

#ifndef __H_UDsUtils__
#define __H_UDsUtils__

#include "CAVDevice.h"

class UDsUtils
{
public:
	UDsUtils() {};
	~UDsUtils() {};

	static void NukeDownstream(IGraphBuilder * inGraph, IBaseFilter * inFilter);
	static void NukeUpstream(IGraphBuilder * inGraph, IBaseFilter * inFilter);
	static BOOL ShowFilterPropertyPage(IBaseFilter * inFilter);

	static IBaseFilter * CreateCamera(CAVDevice& outDevice);
	static IBaseFilter * CreateAudioDevice(CAVDevice& outDevice);

private:
	static IBaseFilter * CreateAVDevice(GUID inCategory, CAVDevice& outDevice);
	static Device_Type DetermineDeviceType(IBaseFilter * inFilter);
};

#endif // __H_UDsUtils__