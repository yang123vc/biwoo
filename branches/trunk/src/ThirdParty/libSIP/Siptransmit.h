// Siptransmit.h file here
#ifndef __Siptransmit_h__
#define __Siptransmit_h__

#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>
#include <boost/thread.hpp>
#include <stl/lockmap.h>
#include <stl/locklist.h>
#include <stl/stldef.h>
#include "SipParameter.h"
#include "libsiphandler.h"


#ifndef HANDLE
typedef void * HANDLE;
#endif // HANDLE

class Siptransmit
{
public:
	Siptransmit(void);
	~Siptransmit(void);

	static tstring GetRemoteIp(int did);
	static tstring GetCaller(int did);
	static int GetRemoteAudioPort(int did);
	static int GetRemoteVideoPort(int did);
	static void GetRemoteMedia(int did);

public:
	bool initSip(const CSipParameter & sipp, OnSipHandler * handler);
	bool isInitSip(void) const {return m_bInitedSip;}
	void quitSip(void);
	const CSipParameter & GetSipParameter(void) const {return m_sipp;}

	int sipRegister(void);					///蛁聊
	void sipUnRegister(void);				///蛁种

	// call control
	int CallInvite(const tstring & callee_num); ///網請
	int CallAnswer(SipCallInfo::pointer callInfo, int localaudioport, int localvideoport);       ///茼湘網請
	int CallTerminate(SipCallInfo::pointer callInfo);                    ///境儂
	int CallSendDtmf(SipCallInfo::pointer callInfo, char dtmf);
	
	void procOneSipEvent(void);
	void addSipEvent(SipEventInfo::pointer eventInfo);

	// SipCallInfo
	SipCallInfo::pointer getCallInfo(int tranId, int callId, int dialogId);
	SipCallInfo::pointer getCallInfo(int callId);

protected:
	int sdp_complete_200ok (int did, osip_message_t * answer, int audio_port, int video_port);
	static void do_proc_SipEvent(Siptransmit * pSiptransmit);
	static void do_proc_EventInfo(Siptransmit * pSiptransmit);

private:
	bool m_bInitedSip;
	int m_regid;
	//osip_message_t * m_osipmessage;
	CSipParameter m_sipp;

	//int  m_LocalAudioPort;
	//int  m_LocalVideoPort;
	bool m_ThreadRunFlag;
	OnSipHandler * m_EvenHandler;
	CLockMap<int, SipCallInfo::pointer> m_mapCallInfo;	// callId->*
	CLockList<SipEventInfo::pointer> m_mapEventInfo;
	boost::thread * m_threadSipEvent;
	boost::thread * m_threadEventInfo;
};

#endif // __Siptransmit_h__

