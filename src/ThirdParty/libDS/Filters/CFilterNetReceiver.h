//
// CFilterNetReceiver.h
//

#ifndef __H_CFilterNetReceiver__
#define __H_CFilterNetReceiver__

#include "CNetOutPin.h"

#include "AVData.h"
#include "stl/LockList.h"
#include <list>
#include "../AVCoding.h"

class OnDoSampleBuffer_
{
public:
	virtual void onAfterSetMediaType(bool bIsVideo) = 0;
//	virtual void onPause(BOOL bIsVideo) = 0;
//	virtual void onRun(BOOL bIsVideo) = 0;
	virtual void onIsOverflow(bool bIsVideo, long nDataSize) = 0;
	virtual int onLastSampleTime(bool bIsVideo, REFERENCE_TIME lastSampleTime) = 0;
};

class CFilterNetReceiver
	: public CBaseFilter
	//, public CMsgStation
{
	friend class CNetOutPin;

private:
	CCritSec			mFilterLock;
	CNetOutPin *		mOutPin;

//	BOOL				mIsVideo;
	IMediaSample *		mSample;

	// Used to set time stamp on audio sample
	long				mAudioBytesPerSecond;
	REFERENCE_TIME		mLastSampleTimeV;
	REFERENCE_TIME		mLastSampleTimeA;

	//
	HANDLE				m_hToSetMediaType;
	HANDLE				m_hKilledOwner;

public:
	CFilterNetReceiver(LPUNKNOWN punk, HRESULT *phr);
	~CFilterNetReceiver();

	void startDoSampleProc(void);
	void stopDoSampleProc(void);

	void setOnDoSampleBuffer(OnDoSampleBuffer_ * handler) {m_hOnDoSampleBuffer = handler;}
	void receiveData(CAVData::pointer receiveData);

public:
	virtual int GetPinCount();
	virtual CBasePin * GetPin(int n);
	STDMETHODIMP Stop();

	void SetAudioBytesPerSecond(long inBytes);
	BOOL GetSampleBuffer(PBYTE * outBuffer);
	IMediaSample * GetMediaSample(void);
	bool DeliverBuffer(const unsigned char * pData, long nLen, unsigned int timestamp);
	//BOOL DeliverHoldingSample(long inSampleSize);
	BOOL DeliverHoldingSample(IMediaSample * pSample, long inSampleSize, unsigned int timestamp);

	// --- public methods ---
	void SetupMediaType(CAVConfigData::pointer configData);
	float GetVideoDuration(void) const {return m_configData != NULL && m_configData->getIsVideo() ? m_configData->getDuration() : (float)0.0;}
	long GetIncrementVideoTime(void) const {return m_configData != NULL && m_configData->getIsVideo() ? m_configData->getFrequency() : 0;}
	long GetIncrementAudioTime(void) const {return m_timeIncrementAudio;}
	void BuildMediaType(void);

protected:
	static DWORD WINAPI doSampleBufferProc(void * pParam);
	void doSampleBuffer(bool bisVideo);

	HANDLE m_hDoSampleProc;
	bool m_bIsStoped;
	//std::list<CData*>	m_listReceived;
	//CCritSec m_lockListData;
	CLockList<CAVData::pointer>	m_listReceived;
	OnDoSampleBuffer_ *	m_hOnDoSampleBuffer;

	CAVConfigData::pointer	m_configData;
	long m_timeIncrementAudio;
	//AVCodingType m_avcoding;

};

#endif // __H_CFilterNetReceiver__