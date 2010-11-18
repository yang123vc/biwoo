//
// CAudioCaptureFilter2.cpp
//

#include <streams.h>
#include <atlstr.h>
#include "CAudioCaptureFilter2.h"

#include "UDsUtils.h"
#include "UFilterUtils.h"

////////////////////////////////////////////////////////////////////////
CAudioCaptureFilter2::CAudioCaptureFilter2(IGraphBuilder * inGraph) : 
CDXFilter(inGraph, GUID_NULL, "Audio Capture")
{
	mDevice = NULL;
}

CAudioCaptureFilter2::~CAudioCaptureFilter2(void)
{
	//mInputList.RemoveAll();
	mInputs.clear();
}

void CAudioCaptureFilter2::SetDevice(CAVDevice * inDevice)
{
	mDevice = inDevice;
}

BOOL CAudioCaptureFilter2::CreateFilter(void)
{
	if (mFilter)
	{
		return TRUE;
	}

	//ASSERT(mDevice);
	if (mGraph)
	{
		mFilter = UDsUtils::CreateAudioDevice(*mDevice);
		if (mFilter)
		{
			if (SUCCEEDED(mGraph->AddFilter(mFilter, mName)))
			{
				BuildInputList();

				//SetDefaultInputPin();
				//SetCaptureBufferSize();
				return TRUE;
			}
		}
	}

	ReleaseFilter();
	return FALSE;
}

// Set microphone as default input
void CAudioCaptureFilter2::SetDefaultInputPin(void)
{
	IEnumPins * pinEnum = NULL;
	if (SUCCEEDED(mFilter->EnumPins(&pinEnum)))
	{
		pinEnum->Reset();

		IPin * pin       = NULL;
		ULONG fetchCount = 0;
		BOOL  found      = FALSE;
		while (!found && SUCCEEDED(pinEnum->Next(1, &pin, &fetchCount)) && 
			fetchCount)
		{
			if (pin)
			{
				PIN_INFO  pinInfo;
				if (SUCCEEDED(pin->QueryPinInfo(&pinInfo)))
				{
					pinInfo.pFilter->Release();

					if (pinInfo.dir == PINDIR_INPUT)
					{
#ifdef UNICODE
						CString  pinName = pinInfo.achName;
#else
						char szName[128];
						::WideCharToMultiByte(CP_ACP, 0, pinInfo.achName,
							-1, szName, 128, NULL, NULL);
						CString  pinName = szName;
#endif
						pinName.MakeLower();
						if (pinName.Find(_T("stereo"), 0) >= 0 || pinName.Find(_T("立体声"), 0) >= 0 || pinName.Find(_T("您听到的声音"), 0) >= 0 || pinName.Find(_T("主录制"), 0) >= 0)
						//if (pinName.Find(_T("stereo"), 0) >= 0 || pinName.Find(_T("立体声"), 0) >= 0)
						{
							IAMAudioInputMixer * pMixer = GetMixer(pin);
							//pMixer->put_Enable(TRUE);
							if (pMixer)
							{
								pMixer->put_MixLevel(0.0);
								pMixer->Release();
							}

						}
						if (pinName.Find(_T("mic"), 0) >= 0 || pinName.Find(_T("麦克风"), 0) >= 0)
						{
							found = TRUE;							
						}else
						{
							found = FALSE;
						}
						// Enable this found pin
						if (found)
						{
							IAMAudioInputMixer * pMixer = GetMixer(pin);
							pMixer->put_Enable(TRUE);
							pMixer->put_MixLevel(1.0);
							pMixer->Release();
							// Save the connector index
							mDevice->SetAudioConnector(GetConnector());
						}
					}
				}
				pin->Release();
			}
		}
		pinEnum->Release();

		// If line-in not found, set the first input item
		if (!found)
		{
			SetConnector(0);
		}
	}
}

void CAudioCaptureFilter2::SetCaptureBufferSize(void)
{
	IPin * pCapturePin = GetPin(FALSE);
	if (pCapturePin)
	{
		DWORD  dwBytesPerSec = 0;
		DWORD  dwLatencyInMilliseconds = 250;//40;
		// Query the current media type used by the capture output pin
		AM_MEDIA_TYPE * pmt = {0};
		IAMStreamConfig * pCfg = NULL;
		HRESULT hr = pCapturePin->QueryInterface(IID_IAMStreamConfig, 
			(void **)&pCfg);
		if (SUCCEEDED(hr))
		{
			hr = pCfg->GetFormat(&pmt);
			if (SUCCEEDED(hr))
			{
				// Fill in values for the new format
				WAVEFORMATEX *pWF = (WAVEFORMATEX *) pmt->pbFormat;
				dwBytesPerSec     = pWF->nAvgBytesPerSec;
				DeleteMediaType(pmt);
			}
			pCfg->Release();
		}

		if (dwBytesPerSec)
		{
			IAMBufferNegotiation * pNeg = NULL;
			hr = pCapturePin->QueryInterface(IID_IAMBufferNegotiation, 
				(void **)&pNeg);
			if (SUCCEEDED(hr))
			{
				ALLOCATOR_PROPERTIES AllocProp;
				int nBytesPerSample = 2;
				int nChannels = 2;
				//AllocProp.cbAlign  = nBytesPerSample * nChannels;  // -1 means no preference.
				AllocProp.cbAlign  = -1;  // -1 means no preference.
				AllocProp.cbBuffer = dwBytesPerSec *  dwLatencyInMilliseconds / 1000;
				AllocProp.cbPrefix = -1;
				AllocProp.cBuffers = -1;
				//AllocProp.cBuffers = 6;
				hr = pNeg->SuggestAllocatorProperties(&AllocProp);
				pNeg->Release();
			}
		}
	}
}

void CAudioCaptureFilter2::BuildInputList(void)
{
	mInputs.clear();

	if (mFilter)
	{
		IEnumPins * pinEnum = NULL;
		if (SUCCEEDED(mFilter->EnumPins(&pinEnum)))
		{
			pinEnum->Reset();

			BOOL pass  = TRUE;
			IPin * pin = NULL;
			ULONG fetchCount = 0;
			while (pass && SUCCEEDED(pinEnum->Next(1, &pin, &fetchCount)) && 
				fetchCount)
			{
				if (pin)
				{
					PIN_INFO pinInfo;
					if (SUCCEEDED(pin->QueryPinInfo(&pinInfo)))
					{
						pinInfo.pFilter->Release();
						if (pinInfo.dir == PINDIR_INPUT)
						{
							CAudioInput inputItem;
#ifdef UNICODE
							inputItem.mInputName = pinInfo.achName;
							CString  pinName = pinInfo.achName;

#else
							char pinNameTemp[128];
							::WideCharToMultiByte(CP_ACP, 0, pinInfo.achName, 
								-1,	pinNameTemp, 128, NULL, NULL);
							inputItem.mInputName = pinNameTemp;
							CString  pinName = pinNameTemp;
#endif

							pinName.MakeLower();
							if (pinName.Find(_T("stereo"), 0) >= 0 || pinName.Find(_T("立体声"), 0) >= 0 || pinName.Find(_T("您听到的声音"), 0) >= 0 || pinName.Find(_T("主录制"), 0) >= 0)
							{
								inputItem.mInputType = CAudioInput::Input_Stereo;
							}else if (pinName.Find(_T("mic"), 0) >= 0 || pinName.Find(_T("麦克风"), 0) >= 0)
							{
								inputItem.mInputType = CAudioInput::Input_MicPhone;
							}

							inputItem.mInputPin  = pin;
							//mInputList.AddTail(inputItem);
							mInputs.push_back(inputItem);
						}
					}
					pin->Release();
					pin = NULL;
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

long CAudioCaptureFilter2::GetConnectorCount(void)
{
	int inputCount  = 0;
	int outputCount = 0;
	UFilterUtils::GetPinCount(mFilter, inputCount, outputCount);
	return inputCount;
}

#ifdef UNICODE
std::wstring CAudioCaptureFilter2::GetConnectorName(long inIndex)
#else
std::string CAudioCaptureFilter2::GetConnectorName(long inIndex)
#endif
{
	if (inIndex >= 0 && inIndex < mInputs.size())
	{
		CAudioInput inputItem = mInputs[inIndex];
		return inputItem.mInputName;
	}
	return _T("");

	/*
	long index   = inIndex;
	POSITION pos = mInputList.GetHeadPosition();
	while (pos && index > 0)
	{
		mInputList.GetNext(pos);
		index--;
	}

	CString connector = _T("");
	if (pos)
	{
		CAudioInput inputItem = mInputList.GetNext(pos);
		connector = inputItem.mInputName;
	}
	return connector;
	*/
}

long CAudioCaptureFilter2::SetConnector(CAudioInput::AudioInputType inputType)
{
	long result = -1;
	for (int index=0; index<mInputs.size(); index++)
	{
		CAudioInput inputItem = mInputs[index];
		if (inputItem.mInputType == inputType)
		{
			IAMAudioInputMixer * pMixer = GetMixer(inputItem.mInputPin);
			pMixer->put_Enable(TRUE);
			pMixer->put_MixLevel(AMF_AUTOMATICGAIN);
			pMixer->Release();
			return index;
			//result = index;
		}
	}
	return result;
}

void CAudioCaptureFilter2::SetConnector(long inIndex)
{
	//if (inIndex >= 0 && inIndex < mInputList.GetCount())
	if (inIndex >= 0 && inIndex < mInputs.size())
	{
		CAudioInput inputItem = mInputs[inIndex];
		IAMAudioInputMixer * pMixer = GetMixer(inputItem.mInputPin);
		pMixer->put_Enable(TRUE);
		pMixer->put_MixLevel(AMF_AUTOMATICGAIN);
		pMixer->Release();
	}
}

long CAudioCaptureFilter2::GetConnector(void)
{
	int index = -1;
	BOOL enabled = FALSE;
	for (index=0; index<mInputs.size(); index++)
	{
		CAudioInput inputItem = mInputs[index];
		IAMAudioInputMixer * pMixer = GetMixer(inputItem.mInputPin);
		pMixer->get_Enable(&enabled);
		pMixer->Release();
		if (enabled)
			break;
	}

/*
	POSITION pos = mInputList.GetHeadPosition();
	while (pos && !enabled)
	{
		CAudioInput inputItem = mInputList.GetNext(pos);
		
		IAMAudioInputMixer * pMixer = GetMixer(inputItem.mInputPin);
		pMixer->get_Enable(&enabled);
		index++;
	}*/
	return (index - 1);
}

IAMAudioInputMixer * CAudioCaptureFilter2::GetMixer(IPin * inPin) const
{
	IAMAudioInputMixer * pMixer = NULL;
	inPin->QueryInterface(IID_IAMAudioInputMixer, (void**) &pMixer);
	if (pMixer)
	{
		// ??
		//pMixer->Release();
	}
	return pMixer;
}

// Set mix level for all input pins
void CAudioCaptureFilter2::SetMixLevel(double inLevel)
{
	BOOL enabled = FALSE;
	for (int index=0; index<mInputs.size(); index++)
	{
		CAudioInput inputItem = mInputs[index];
		IAMAudioInputMixer * pMixer = GetMixer(inputItem.mInputPin);
		pMixer->get_Enable(&enabled);
		if (enabled)
			pMixer->put_MixLevel(inLevel);
		pMixer->Release();
		if (enabled)
			break;
	}
}

double CAudioCaptureFilter2::GetMixLevel(void) const
{
	BOOL enabled = FALSE;
	double mixLevel = -1.0;
	for (int index=0; index<mInputs.size(); index++)
	{
		CAudioInput inputItem = mInputs[index];
		IAMAudioInputMixer * pMixer = GetMixer(inputItem.mInputPin);
		pMixer->get_Enable(&enabled);
		if (enabled)
		{
			pMixer->get_MixLevel(&mixLevel);
		}
		pMixer->Release();
		if (enabled)
			break;
	}

	return mixLevel;
}