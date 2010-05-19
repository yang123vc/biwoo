//
// UDsUtils.cpp
//

#include <streams.h>
#include "UDsUtils.h"
#include "UFilterUtils.h"

///////////////////////////////////////////////////////////////////////////////
// Only USB camera or DV camcorder will be created
IBaseFilter * UDsUtils::CreateCamera(CAVDevice& outDevice)
{
	return CreateAVDevice(CLSID_VideoInputDeviceCategory, outDevice);
}

// Create the first found audio device
IBaseFilter * UDsUtils::CreateAudioDevice(CAVDevice& outDevice)
{
	return CreateAVDevice(CLSID_AudioInputDeviceCategory, outDevice);
}

// Create the first found audio device
IBaseFilter * UDsUtils::CreateAVDevice(GUID inCategory, CAVDevice& outDevice)
{
	IBaseFilter * pDevice = NULL;

	ICreateDevEnum * enumHardware = NULL;
    if (SUCCEEDED(CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_ALL,
		IID_ICreateDevEnum, (void**) &enumHardware)))
	{
		IEnumMoniker * enumMoniker = NULL;
		if (SUCCEEDED(enumHardware->CreateClassEnumerator(inCategory, 
			&enumMoniker, 0)))
		{
			if (enumMoniker)
			{
				enumMoniker->Reset();

				ULONG cFetched = 0;
				IMoniker * moniker = NULL;
				while (SUCCEEDED(enumMoniker->Next(1, &moniker, &cFetched)) && 
					cFetched && !pDevice)
				{
					if (moniker)
					{
						WCHAR *wzDisplayName = NULL;
						// Get display name
						if (SUCCEEDED (moniker->GetDisplayName(NULL, NULL, &wzDisplayName)))
						{
							char displayName[1024];
							WideCharToMultiByte(CP_ACP, 0, wzDisplayName, -1, displayName, 1024, "", NULL);
							CoTaskMemFree(wzDisplayName);

							IPropertyBag * propertyBag = NULL;
							if (SUCCEEDED(moniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&propertyBag)))
							{
								VARIANT name;
								name.vt = VT_BSTR;
								// Get friendly name
								if (SUCCEEDED(propertyBag->Read(L"FriendlyName", &name, NULL)))
								{
									char friendlyName[256];
									friendlyName[0] = 0;
									WideCharToMultiByte(CP_ACP, 0, name.bstrVal, -1,
										friendlyName, 80, NULL, NULL);

									IBaseFilter * filter = NULL;
									if (SUCCEEDED(moniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&filter)))
									{
										Device_Type type = DetermineDeviceType(filter);
										if (type != DT_Unknown)
										{
											// Save this device info
											outDevice.SetDeviceDisplayName(displayName);
											outDevice.SetDeviceFriendlyName(friendlyName);
											outDevice.SetDevideType(type);
											pDevice = filter;
											pDevice->AddRef();
										}
										filter->Release();
									}
								}
								propertyBag->Release();
							}
						}
						moniker->Release();
					}
				}
				enumMoniker->Release();
			}
		}
		enumHardware->Release();
	}

	return pDevice;
}

Device_Type UDsUtils::DetermineDeviceType(IBaseFilter * inFilter)
{
	// Check if DV device
	IAMExtTransport * pAMExtTransPost = NULL;
	inFilter->QueryInterface(IID_IAMExtTransport, (void **)&pAMExtTransPost);
	if (pAMExtTransPost)
	{
		pAMExtTransPost->Release();
		return DT_DV;
	}

	// Check if WDM analog
	// USB camera usually has only one output pin
	IAMAnalogVideoDecoder * pDecoder = NULL;
	IAMCameraControl *      pCamera  = NULL;
	IAMVideoProcAmp *		pAmp	 = NULL;
	inFilter->QueryInterface(IID_IAMAnalogVideoDecoder, (void **)&pDecoder);
	inFilter->QueryInterface(IID_IAMCameraControl, (void **)&pCamera);
	inFilter->QueryInterface(IID_IAMVideoProcAmp, (void **)&pAmp);
	if (pDecoder || pCamera || pAmp)
	{
		SAFE_RELEASE(pDecoder);
		SAFE_RELEASE(pCamera);
		SAFE_RELEASE(pAmp);
		// Further checking for USB camema
		int inputs = 0, outputs = 0;
		UFilterUtils::GetPinCount(inFilter, inputs, outputs);
		//if (inputs + outputs <= 2)
		if (inputs + outputs <= 3)		// ???
		{
			return DT_USB_Camera;
		}
		else
		{
			return DT_Unknown;
		}
	}

	// Check if audio capture device
	IAMAudioInputMixer * pAudioMixer = NULL;
	inFilter->QueryInterface(IID_IAMAudioInputMixer, (void **)&pAudioMixer);
	if (pAudioMixer)
	{
		pAudioMixer->Release();
		return DT_Audio_Capture;
	}
	 
	return DT_Unknown;
}

void UDsUtils::NukeDownstream(IGraphBuilder * inGraph, IBaseFilter * inFilter) 
{
	if (inGraph && inFilter)
	{
		IEnumPins * pinEnum = NULL;
		if (SUCCEEDED(inFilter->EnumPins(&pinEnum)))
		{
			pinEnum->Reset();

			IPin * pin     = NULL;
			ULONG cFetched = 0;
			BOOL pass      = TRUE;
			while (pass && SUCCEEDED(pinEnum->Next(1, &pin, &cFetched)))
			{
				if (pin && cFetched)
				{
					IPin * connectedPin = 0;
					pin->ConnectedTo(&connectedPin);
					if (connectedPin) 
					{
						PIN_INFO pininfo;
						if (SUCCEEDED(connectedPin->QueryPinInfo(&pininfo)))
						{
							pininfo.pFilter->Release();
							if (pininfo.dir == PINDIR_INPUT) 
							{
								NukeDownstream(inGraph, pininfo.pFilter);
								inGraph->Disconnect(connectedPin);
								inGraph->Disconnect(pin);
								inGraph->RemoveFilter(pininfo.pFilter);
							}
						}
						connectedPin->Release();
					}
					pin->Release();
				}
				else
				{
					pass = FALSE;
				}
			}
			pinEnum->Release();
		}
	}
}

void UDsUtils::NukeUpstream(IGraphBuilder * inGraph, IBaseFilter * inFilter) 
{
	if (inGraph && inFilter)
	{
		IEnumPins * pinEnum = NULL;
		if (SUCCEEDED(inFilter->EnumPins(&pinEnum)))
		{
			pinEnum->Reset();

			IPin * pin     = NULL;
			ULONG cFetched = 0;
			BOOL pass      = TRUE;
			while (pass && SUCCEEDED(pinEnum->Next(1, &pin, &cFetched)) && cFetched)
			{
				if (pin)
				{
					IPin * connectedPin = 0;
					pin->ConnectedTo(&connectedPin);
					if(connectedPin) 
					{
						PIN_INFO pininfo;
						if (SUCCEEDED(connectedPin->QueryPinInfo(&pininfo)))
						{
							if(pininfo.dir == PINDIR_OUTPUT) 
							{
								NukeUpstream(inGraph, pininfo.pFilter);
								inGraph->Disconnect(connectedPin);
								inGraph->Disconnect(pin);
								inGraph->RemoveFilter(pininfo.pFilter);
							}
							pininfo.pFilter->Release();
						}
						connectedPin->Release();
					}
					pin->Release();
				}
				else
				{
					pass = FALSE;
				}
			}
			pinEnum->Release();
		}
	}
}

BOOL UDsUtils::ShowFilterPropertyPage(IBaseFilter * inFilter)
{
	if (!inFilter)
	{
		return FALSE;
	}

	ISpecifyPropertyPages * pSpecify;
	HRESULT hr = inFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpecify);
	if (SUCCEEDED(hr))
	{
		CAUUID   caGUID;
		pSpecify->GetPages(&caGUID);
		pSpecify->Release();

		OleCreatePropertyFrame(
			::GetActiveWindow(),        // Parent window
			0,                          // x (Reserved)
			0,                          // y (Reserved)
			NULL,                       // Caption for the dialog box
			1,                          // Number of filters
			(IUnknown **) &inFilter,    // Pointer to the filter
			caGUID.cElems,              // Number of property pages
			caGUID.pElems,              // Pointer of property page CLSIDs
			0,                          // Locale identifier
			0,                          // Reserved
			NULL                        // Reserved
			);
		CoTaskMemFree(caGUID.pElems);
		return TRUE;
	}
	return FALSE;
}