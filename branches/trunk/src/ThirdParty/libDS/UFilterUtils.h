//
// UFilterUtils.h
//

#ifndef __H_UFilterUtils__
#define __H_UFilterUtils__

typedef enum
{
	PIN_VIDEO,
	PIN_AUDIO,
	PIN_STREAM,
	PIN_INTERLEAVED,
	PIN_UNKNOWN
} PIN_MEDIA;

class UFilterUtils
{
public:
	UFilterUtils() {};
	~UFilterUtils() {};

	// NOT outstanding reference
	static IPin * GetPin(IBaseFilter * inFilter, BOOL inInput, 
		const char * inPartialName = NULL);
	static IPin * GetUnconnectedPin(IBaseFilter * inFilter, BOOL inInput);
	static BOOL GetPinCount(IBaseFilter * inFilter, int & outInputPin, 
		int & outOutputPin);
	static BOOL IsCompletelyConnected(IBaseFilter * inFilter);

	// Determint the pin type: audio, video, interleaved or stream
	static PIN_MEDIA GetPinMediaType(IPin * inPin);
	static BOOL HasMediaType(IPin * inPin, GUID inMajorType, GUID inSubType);
};

#endif // __H_UFilterUtils__