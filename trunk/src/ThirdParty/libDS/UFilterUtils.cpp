//
// UFilterUtils.cpp
//

/*-----------------------------------------------------*\
			HQ Tech, Make Technology Easy!       
 More information, please go to http://hqtech.nease.net.
/*-----------------------------------------------------*/

#include <streams.h>
#include "UFilterUtils.h"


////////////////////////////////////////////////////////////////////////////
IPin * UFilterUtils::GetPin(IBaseFilter * inFilter, BOOL inInput, 
							const char * inPartialName)
{
	PIN_DIRECTION direction = inInput ? PINDIR_INPUT : PINDIR_OUTPUT;
	IPin * foundPin = NULL;

	if (inFilter)
	{
		IEnumPins * pinEnum = NULL;
		if (SUCCEEDED(inFilter->EnumPins(&pinEnum)))
		{
			pinEnum->Reset();

			IPin * pin = NULL;
			ULONG fetchCount = 0;
			while (!foundPin && SUCCEEDED(pinEnum->Next(1, &pin, &fetchCount)) && fetchCount)
			{
				if (pin)
				{
					PIN_INFO pinInfo;
					if (SUCCEEDED(pin->QueryPinInfo(&pinInfo)))
					{
						if (pinInfo.dir == direction)
						{
							// Ignore the pin name
							if (!inPartialName)
							{
								pin->AddRef();
								foundPin = pin;
							}
							else
							{
								char pinName[128];
								::WideCharToMultiByte(CP_ACP, 0, pinInfo.achName, 
									-1,	pinName, 128, NULL, NULL);
								if (::strstr(pinName, inPartialName))
								{
									pin->AddRef();
									foundPin = pin;
								}
							}
						}
						pinInfo.pFilter->Release();
					}
					pin->Release();
				}
			}
			pinEnum->Release();
		}
	}

	// We don't keep outstanding reference count
	if (foundPin)
	{
		foundPin->Release();
	}
	return foundPin;
}

IPin * UFilterUtils::GetUnconnectedPin(IBaseFilter * inFilter, BOOL inInput)
{
	PIN_DIRECTION direction = inInput ? PINDIR_INPUT : PINDIR_OUTPUT;
	IPin * foundPin = NULL;

	if (inFilter)
	{
		IEnumPins * pinEnum = NULL;
		if (SUCCEEDED(inFilter->EnumPins(&pinEnum)))
		{
			pinEnum->Reset();

			IPin * pin = NULL;
			ULONG fetchCount = 0;
			while (!foundPin && SUCCEEDED(pinEnum->Next(1, &pin, &fetchCount)) && fetchCount)
			{
				if (pin)
				{
					PIN_INFO pinInfo;
					if (SUCCEEDED(pin->QueryPinInfo(&pinInfo)))
					{
						pinInfo.pFilter->Release();
						if (pinInfo.dir == direction)
						{
							IPin * connected = NULL;
							pin->ConnectedTo(&connected);
							if (connected)
							{
								connected->Release();
							}
							else
							{
								pin->AddRef();
								foundPin = pin;
							}
						}						
					}
					pin->Release();
				}
			}
			pinEnum->Release();
		}
	}

	// We don't keep outstanding reference count
	if (foundPin)
	{
		foundPin->Release();
	}
	return foundPin;
}

BOOL UFilterUtils::GetPinCount(IBaseFilter * inFilter, 
							   int & outInputPin, 
							   int & outOutputPin)
{
	outInputPin  = 0;
	outOutputPin = 0;

	if (inFilter)
	{		
		IEnumPins * pinEnum = NULL;
		if (SUCCEEDED(inFilter->EnumPins(&pinEnum)))
		{
			pinEnum->Reset();

			IPin * pin = NULL;
			ULONG fetchCount = 0;
			while (SUCCEEDED(pinEnum->Next(1, &pin, &fetchCount)) && fetchCount)
			{
				if (pin)
				{
					PIN_INFO  pinInfo;
					if (SUCCEEDED(pin->QueryPinInfo(&pinInfo)))
					{
						pinInfo.pFilter->Release();
						if (pinInfo.dir == PINDIR_INPUT)
						{
							outInputPin++;
						}
						else
						{
							outOutputPin++;
						}
					}
					pin->Release();
				}
			}
			pinEnum->Release();
		}
		return TRUE;
	}

	return FALSE;
}

BOOL UFilterUtils::IsCompletelyConnected(IBaseFilter * inFilter)
{
	if (!inFilter)
	{
		return FALSE;
	}

	BOOL allConnected   = TRUE;
	IEnumPins * pinEnum = NULL;
	if (SUCCEEDED(inFilter->EnumPins(&pinEnum)))
	{
		pinEnum->Reset();

		IPin * pin = NULL;
		ULONG fetchCount = 0;
		while (SUCCEEDED(pinEnum->Next(1, &pin, &fetchCount)) && 
			fetchCount && allConnected)
		{
			if (pin)
			{
				pin->Release();

				IPin * connected = NULL;
				pin->ConnectedTo(&connected);
				if (connected)
				{
					connected->Release();
				}
				else
				{
					allConnected = FALSE;
				}				
			}
		}
		pinEnum->Release();
	}

	return allConnected;
}

// Pick up one of media types to determine pin type
PIN_MEDIA UFilterUtils::GetPinMediaType(IPin * inPin)
{
	PIN_MEDIA pinMedia = PIN_UNKNOWN;

	if (inPin)
	{
		IEnumMediaTypes * typeEnum = NULL;
		if (SUCCEEDED(inPin->EnumMediaTypes(&typeEnum)))
		{
			typeEnum->Reset();

			AM_MEDIA_TYPE * type = 0;
			ULONG     fetchCount = 0;
			if (SUCCEEDED(typeEnum->Next(1, &type, &fetchCount)) && fetchCount)
			{
				if (type)
				{
					if (type->majortype == MEDIATYPE_Video)
					{
						pinMedia = PIN_VIDEO;
					}
					else if (type->majortype == MEDIATYPE_Audio)
					{
						pinMedia = PIN_AUDIO;
					}
					else if (type->majortype == MEDIATYPE_Interleaved)
					{
						pinMedia = PIN_INTERLEAVED;
					}
					else if (type->majortype == MEDIATYPE_Stream)
					{
						pinMedia = PIN_STREAM;
					}

					DeleteMediaType(type);
					type = NULL;
				}
			}
			typeEnum->Release();
		}
	}

	return pinMedia;
}

BOOL UFilterUtils::HasMediaType(IPin * inPin, GUID inMajorType, GUID inSubType)
{
	return FALSE;
}
