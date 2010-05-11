//
// CVideoCaptureFilter2.h
//

#ifndef __H_CVideoCaptureFilter2__
#define __H_CVideoCaptureFilter2__

#include "CDXFilter.h"

class CAVDevice;
class CVideoCaptureFilter2 : public CDXFilter  
{
private:
	CAVDevice *		mDevice;

public:
	CVideoCaptureFilter2(IGraphBuilder * inGraph);
	virtual ~CVideoCaptureFilter2(void);

	void SetDevice(CAVDevice * inDevice);

	virtual BOOL CreateFilter(void);
	virtual void ReleaseFilter(void);

	long GetResolution(void);

	IAMAnalogVideoDecoder * GetAnalogDecoder(void);

	void SetResolution(void);
	void AdjustOutput(IPin * pPin);
};

#endif // __H_CVideoCaptureFilter2__
