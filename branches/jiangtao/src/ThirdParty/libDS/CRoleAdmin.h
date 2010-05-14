//
// CRoleAdmin.h
//

#ifndef __H_CRoleAdmin__
#define __H_CRoleAdmin__

#include <boost/thread.hpp>
#include <stl/locklist.h>
#include <libG729a/libG729a.h>
#include <libCoder/libCoder.h>
#include <libcoder/H264Encoder.h>
//#include "AviFileWrite.h"

#include "CDXGraph.h"
#include "CPreviewAdmin.h"
#include "CFilterNetReceiver.h"
#include "CFilterNetSender.h"

#include "libdshandler.h"
#include "AVData.h"
#include "AVParameter.h"
#include "globaldefs.h"
#include "AVCodecProxy.h"
#include "XvidProxy.h"

#include <list>

#define USE_AVIREADERFILTER		0	// [0,1]

class CRoleAdmin
	: public CPreviewAdmin
	, public OnDoSampleBuffer_
	//, public AVParameters
	, public CAVParameter
	//, public OnDSHandler
	, public DoDSHandler
	, public OnDSReceiveEvent

{
protected:
	long			mDeviceConfig;
	
	CDXGraph *		mRemoteVideoGraph; // Receiving
	CDXGraph *		mRemoteAudioGraph; 
	CDXGraph *		mLocalVideoGraph;  // Sending
	CDXGraph *		mLocalAudioGraph;
	CDXGraph *      mLocalPreviewGraph; //PreviewGraph

	CFilterNetReceiver *	mVideoReceiver;
	CFilterNetReceiver *    mVideoPreviewReceiver;
	CFilterNetReceiver *	mAudioReceiver;
	CFilterNetSender *		mVideoSender;
	CFilterNetSender *		mAudioSender;

	CDXFilter *		m_filterAVIMux;
	CDXFilter *		m_filterFileWriter;
	CDXFilter *		m_filterInfTee1;	// for video
	CDXFilter *		m_filterInfTee2;	// for audio

	CDXFilter *		mVideoPreviewDecoderFilter;
	CDXFilter *		m_filterAVISplitter;
	CDXFilter *		m_filterAsyncFileReader;

	CDXFilter *		m_filterMP3Enc;
	CDXFilter *		m_filterMP3Dec;

	CDXFilter *		mVideoPreviewRenderer;

	CDXFilter *		m_filterMainConpectMPEGSplitter;

public:
	CRoleAdmin();
	virtual ~CRoleAdmin();

	// audio parameters
//	void setAudioParams(int nChannels=2, int nFrequency=44100);
//	void setVideoParams(int nFrequency=500000, int nWidth=320, int nHeight=240);

	// handler
	void receiveConfig(CAVConfigData::pointer configData);
	void receiveData(CAVData::pointer data);
	void setSenderHandler(OnDSHandler * handler);

	void SetRemoteVideoWindow(HWND inWindow);
	void SetRecordFilename(const WCHAR * recordFilename);
	void SetReadFilename(const WCHAR * readFilename);
	// Has audio? Has Video? (local and remote)
	void SetDeviceConfig(long inConfig);
	virtual void Disconnect(BOOL inNotifyPeer = TRUE);

	//CDXGraph * getRemoteVideoGraph(void) {return mRemoteVideoGraph;}
	//CDXGraph * getLocalVideoGraph(void) {return mLocalVideoGraph;}

	static bool findVideoDev(void);
	static bool findAudioDev(void);

	void doProcAVData(void);

protected:
	void CompleteVideoReceivingGraph(void);
	void CompleteAudioReceivingGraph(void);

	virtual BOOL CreateGraph(void);
	virtual void DeleteGraph(void);

	virtual BOOL CreateInputFilters(void);
	virtual void ReleaseInputFilters(void);

	virtual BOOL CreateOutputFilters(void);
	virtual void ReleaseOutputFilters(void);

	virtual BOOL ConnectFilters(void);
	virtual BOOL AfterBuildGraph(void);

	// for OnDoSampleBuffer_
	virtual void onAfterSetMediaType(bool bIsVideo);
//	virtual void onPause(BOOL bIsVideo);
//	virtual void onRun(BOOL bIsVideo);
	virtual void onIsOverflow(bool bIsVideo, long nDataSize);
	virtual int onLastSampleTime(bool bIsVideo, REFERENCE_TIME lastSampleTime);

	// OnDSReceiveEvent
	virtual void onReceive(const unsigned char * data, long size, unsigned int timestamp, bool is_video);

	// DoDSHandler handler
	virtual void SetOnDSHandler(OnDSHandler * handler) {setSenderHandler(handler);}
	virtual void ClearOnDSHandler(void) {setSenderHandler(NULL);}

	virtual void PlayLocal(void);
	virtual void PauseLocal(void);
	virtual void StopLocal(void);
	virtual void PlayRemote(void);
	virtual void PauseRemote(void);
	virtual void StopRemote(void);
	virtual void StopAll(void);
	virtual void RemoteVideoReversal(void) {this->reversal(!this->reversal());}

	virtual bool SetRemoteAudioVolume(long volume) {return mRemoteAudioGraph == NULL ? false : mRemoteAudioGraph->SetAudioVolume(volume);}
	virtual long GetRemoteAudioVolume(void) const {return mRemoteAudioGraph == NULL ? 0 : mRemoteAudioGraph->GetAudioVolume();}
	virtual bool SetRemoteAudioBalance(long balance) {return mRemoteAudioGraph == NULL ? false : mRemoteAudioGraph->SetAudioBalance(balance);}
	virtual long GetRemoteAudioBalance(void) const {return mRemoteAudioGraph == NULL ? 0 : mRemoteAudioGraph->GetAudioBalance();}

	virtual bool GetRemoteFullScreen(void) const {return mRemoteVideoGraph == NULL ? false : mRemoteVideoGraph->GetFullScreen();}
	virtual bool SetRemoteFullScreen(bool enable) {return mRemoteVideoGraph == NULL ? false : mRemoteVideoGraph->SetFullScreen(enable);}
	virtual bool GetLocalFullScreen(void) const {return mLocalVideoGraph == NULL ? false : mLocalVideoGraph->GetFullScreen();}
	virtual bool SetLocalFullScreen(bool enable) {return mLocalVideoGraph == NULL ? false : mLocalVideoGraph->SetFullScreen(enable);}
	virtual void MoveWindow(void) {ReSizeScreen();}

	virtual bool ReSizeScreen(void)
	{
		if (mLocalPreviewGraph != NULL)
		{
			mLocalPreviewGraph->moveWindow();
		}

		if (mLocalVideoGraph)
		{
			mLocalVideoGraph->moveWindow();
		}
		if (mRemoteVideoGraph)
		{
			mRemoteVideoGraph->moveWindow();
		}
		return true;
	}
	virtual void ReceiveRemoteData(const unsigned char * data, long size, unsigned int timestamp, bool is_video);

	static void do_procAVData(CRoleAdmin * pOwner);
private:
	OnDSHandler * m_handlerSender;
	HANDLE m_hCompletedVideoGraph;
	bool m_bKilledOwner;

	bool mIsVideoMediaTypeSent;
	bool mIsAudioMediaTypeSent;
	long m_nVideoOverflowData;	//
	long m_nAudioOverflowData;
	HWND m_wndRemoteVideoWindow;
	bool m_bIsRemotePaused;
	std::wstring m_recordFilename;	// for record
	std::wstring m_readFilename;	// for splitter
/*
	// audio parameters
	int m_nChannels;		// default 2
	int m_nFrequency;		// default 44100
	// video parameters
	int m_nFrameRate;		// default 25
	int m_nWidth;			// default 320
	int m_nHeight;			// default 240
*/
	///////////////////////////////////
	// ÊÓÆµ¼ÆÊý
	int m_nVideoCount;
	REFERENCE_TIME m_lastSampleAudioTime;

	CH264Encoder m_H264Codec;
	CG729a m_g729a;
	CAVCodecProxy m_audioCodec;
	CAVCodecProxy m_videoCodec;
	CXvidProxy m_xvid;
	
	//CAviFileWrite m_AviFile;
	boost::thread * m_procAVData;
	CLockList<CAVData::pointer> m_avdatas;
	//
	int m_MediaSubtype;
};

#endif // __H_CRoleAdmin__