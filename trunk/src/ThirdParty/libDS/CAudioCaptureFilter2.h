//
// CAudioCaptureFilter2.h
//

#ifndef __H_CAudioCaptureFilter2__
#define __H_CAudioCaptureFilter2__

#include <string>
#include <vector>
#include "CDXFilter.h"

class CAudioInput
{
public:
	enum AudioInputType
	{
		Input_MicPhone,
		Input_Stereo
	};

#ifdef UNICODE
	std::wstring	mInputName;
#else
	std::string		mInputName;
#endif
	IPin *			mInputPin;   // NOT outstanding reference

	AudioInputType mInputType;

	CAudioInput() {};
	~CAudioInput() {};
};

//typedef CList<CAudioInput, CAudioInput&>  INPUT_LIST;
typedef std::vector<CAudioInput>  INPUT_VECTOR;

class CAVDevice;
class CAudioCaptureFilter2 : public CDXFilter  
{
private:
	INPUT_VECTOR	mInputs;
	CAVDevice *		mDevice;

public:
	CAudioCaptureFilter2(IGraphBuilder * inGraph);
	virtual ~CAudioCaptureFilter2(void);

	void SetDevice(CAVDevice * inDevice);
	virtual BOOL CreateFilter(void);

	long GetConnectorCount(void);
	const INPUT_VECTOR & GetInputs(void) const {return mInputs;}

#ifdef UNICODE
	std::wstring GetConnectorName(long inIndex);
#else
	std::string GetConnectorName(long inIndex);
#endif
	long SetConnector(CAudioInput::AudioInputType inputType);
	void SetConnector(long inIndex);
	long GetConnector(void);

	void SetMixLevel(double inLevel);
	double GetMixLevel(void) const;

	void SetCaptureBufferSize(void);

private:
	void SetDefaultInputPin(void);

	void BuildInputList(void);
	IAMAudioInputMixer * GetMixer(IPin * inPin) const;
};

#endif // __H_CAudioCaptureFilter2__
