//
// CFilterNetSender.cpp
//

#include <streams.h>          // quartz, includes windows
#include <initguid.h>

#include "CFilterNetSender.h"

// {A7F18A36-2899-4546-8D88-4D21D4F2DA4D}
DEFINE_GUID(CLSID_NetSender, 
0xa7f18a36, 0x2899, 0x4546, 0x8d, 0x88, 0x4d, 0x21, 0xd4, 0xf2, 0xda, 0x4d);

// ----------------------------------------------------------------------------
//            Filter implementation
// ----------------------------------------------------------------------------
CFilterNetSender::CFilterNetSender(LPUNKNOWN lpunk, HRESULT *phr) :
CBaseFilter(NAME("NetSender"), lpunk, &mFilterLock, CLSID_NetSender)
{
	mPosition = NULL;
	mInputPin = new CXInputPin(this, phr);
} 

CFilterNetSender::~CFilterNetSender()
{
	delete mInputPin;
	if (mPosition != NULL)
	{
		delete mPosition;
		mPosition = NULL;
	}
}

void CFilterNetSender::SetIsLiveMedia(bool isLiveMedia)
{
	if (mInputPin)
		mInputPin->SetIsLiveMedia(isLiveMedia);
}

//
// Basic COM - used here to reveal our own interfaces
STDMETHODIMP CFilterNetSender::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) 
	{
		// Expose mediaseeking interface on our renderer
		HRESULT hr = S_OK;
		if (mPosition == NULL) 
		{
			mPosition  = new CPosPassThru(NAME("Pass Through"),
										(IUnknown *) GetOwner(),
										(HRESULT *) &hr, mInputPin);
			if (mPosition == NULL) 
			{
				return E_OUTOFMEMORY;
			}
		}
		if (FAILED(hr)) 
		{
			delete mPosition;
			mPosition = NULL;
			return hr;
		}
		return mPosition->NonDelegatingQueryInterface(riid, ppv);
	}
	else
	{
		return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
	}
} 

// Instantiate the pin object
CBasePin * CFilterNetSender::GetPin(int n)
{
	if (n == 0) 
	{
		return mInputPin;
	}
	else 
	{
		return NULL;
	}
} 

// We have only one rendered input pin
int CFilterNetSender::GetPinCount()
{
	return 1;
}

/*
STDMETHODIMP CFilterNetSender::Stop()
{
	return CBaseFilter::Stop();
}
*/

// --- public methods ---
void CFilterNetSender::SetHandler(OnDSReceiveEvent * handler)
{
	if (mInputPin)
	{
		mInputPin->SetHandler(handler);
	}
}

//void CFilterNetSender::SetCoding(AVCodingType avc)
//{
//	if (mInputPin)
//	{
//		mInputPin->SetCoding(avc);
//	}
//}
