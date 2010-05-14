//
// CFilterNetSender.h
//

#ifndef __H_CFilterNetSender__
#define __H_CFilterNetSender__

#include "CXInputPin.h"

class CFilterNetSender : public CBaseFilter
{
	friend class CXInputPin;

protected:
	CCritSec			mFilterLock;
	CXInputPin *		mInputPin;
	CPosPassThru *		mPosition;    // Renderer position controls

public:
	CFilterNetSender(LPUNKNOWN lpunk, HRESULT *phr);
	~CFilterNetSender();

	void SetIsLiveMedia(bool isLiveMedia);

	DECLARE_IUNKNOWN;
	// Basic COM - used here to reveal our own interfaces
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
	// Pin enumeration
	CBasePin * GetPin(int n);
	int GetPinCount();
//	STDMETHODIMP Stop();

    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q) {return E_NOTIMPL;}

	// --- public methods ---
	void SetHandler(OnDSReceiveEvent * handler);
	//void SetCoding(AVCodingType avc);
};

#endif // __H_CFilterNetSender__