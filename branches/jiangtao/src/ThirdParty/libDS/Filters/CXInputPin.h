//
// CXInputPin.h
//

#ifndef __H_CMemDestInputPin__
#define __H_CMemDestInputPin__

#include "GlobalDefs.h"
//#include "libdshandler.h"
//#include "AVCoding.h"

class OnDSReceiveEvent
{
public:
	virtual void onReceive(const unsigned char * data, long size, unsigned int timestamp, bool is_video) = 0;

};

class CFilterNetSender;
class CXInputPin : public CRenderedInputPin
{
	friend class CFilterNetSender;

private:
	CFilterNetSender *	mFilter;
	CCritSec			mReceiveLock;
	BOOL				mIsMediaTypeSent;

public:
	CXInputPin(CFilterNetSender *pFilter, HRESULT *phr);
	~CXInputPin();

	// Filter delegating methods
	void SetHandler(OnDSReceiveEvent * handler) {m_handler = handler;}
	void SetIsLiveMedia(bool isLiveMedia) {m_bIsLiveMedia = isLiveMedia;}
//	void CancelPendingSend(void);

	// Do something with this media sample
	STDMETHODIMP Receive(IMediaSample *pSample);
	STDMETHODIMP EndOfStream(void);
	STDMETHODIMP BeginFlush(void);
	STDMETHODIMP EndFlush(void);
	STDMETHODIMP ReceiveCanBlock(void);

#if (USES_H264CODEC)
//    virtual HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);
#endif // USES_H264CODEC
	// Check if the pin can support this specific proposed type and format
	HRESULT CheckMediaType(const CMediaType * inMediaType);
	STDMETHODIMP Notify(IBaseFilter *pSender, Quality q){return E_NOTIMPL;}

private:
	OnDSReceiveEvent * m_handler;
	LONGLONG preEndTime;  ///增加全局变量
	REFERENCE_TIME m_lastEndTime;

	bool	m_bIsLiveMedia;	// true: livemedia; false: read from file.
};

#endif // __H_CMemDestInputPin__