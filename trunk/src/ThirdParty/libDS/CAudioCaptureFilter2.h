//
// CAudioCaptureFilter2.h
//

#ifndef __H_CAudioCaptureFilter2__
#define __H_CAudioCaptureFilter2__

#include <string>
#include <list>
#include "CDXFilter.h"

class CAudioInput
{
public:
#ifdef UNICODE
	std::wstring	mInputName;
#else
	std::string		mInputName;
#endif
	IPin *			mInputPin;   // NOT outstanding reference

	CAudioInput() {};
	~CAudioInput() {};
};

//typedef CList<CAudioInput, CAudioInput&>  INPUT_LIST;
typedef std::list<CAudioInput>  INPUT_LIST;

class CAVDevice;
class CAudioCaptureFilter2 : public CDXFilter  
{
private:
	INPUT_LIST		mInputList;
	CAVDevice *		mDevice;

public:
	CAudioCaptureFilter2(IGraphBuilder * inGraph);
	virtual ~CAudioCaptureFilter2(void);

	void SetDevice(CAVDevice * inDevice);
	virtual BOOL CreateFilter(void);

	long GetConnectorCount(void);
#ifdef UNICODE
	std::wstring GetConnectorName(long inIndex);
#else
	std::string GetConnectorName(long inIndex);
#endif
	void SetConnector(long inIndex);
	long GetConnector(void);
	void SetMixLevel(double inLevel);

	void SetCaptureBufferSize(void);

private:
	void SetDefaultInputPin(void);

	void BuildInputList(void);
	IAMAudioInputMixer * GetMixer(IPin * inPin);
};

#endif // __H_CAudioCaptureFilter2__
