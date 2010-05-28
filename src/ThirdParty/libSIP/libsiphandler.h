// libsiphandler.h file here
#ifndef __libsiphandler_h__
#define __libsiphandler_h__

#include "SipCallInfo.h"
#include "SipParameter.h"

class SipEventInfo
{
public:
	enum EventType
	{
		// 注册状态: 1 开始 2 成功 3 失败 4 终止
		ET_Register = 0x01

		// 收到呼叫
		, ET_CallInvite = 0x21

		// 正在尝试呼叫对方...
		, ET_CallProceeding = 0x22

		// 对方振铃
		, ET_CallRinging = 0x23

		// 呼叫失败: 对方忙或者拒绝
		, ET_CallRequestFailure = 0x24

		// 对方摘机
		, ET_CallAnswered = 0x25

		// 呼叫消息（如DTMF）
		, ET_CallMessageNew = 0x31

		// 呼叫结束（挂机）
		, ET_CallClosed = 0x26

		, ET_Other = 0xFF
	};

	typedef boost::shared_ptr<SipEventInfo> pointer;
	
	static SipEventInfo::pointer create(EventType et, int eventData)
	{
		return SipEventInfo::pointer(new SipEventInfo(et, eventData));
	}
	static SipEventInfo::pointer create(EventType et, SipCallInfo::pointer callInfo)
	{
		return SipEventInfo::pointer(new SipEventInfo(et, callInfo));
	}
	EventType getEventType(void) const {return m_et;}
	SipCallInfo::pointer getCallInfo(void) const {return m_callInfo;}

	///////////////////////////////////////////////////////////////////////////
	// 注册事件:
	// 1 开始 2 成功 3 失败 4 终止
	int getEventData(void) const {return m_eventData;}

public:
	SipEventInfo(EventType et, int eventData)
		: m_et(et), m_eventData(eventData)
	{}
	SipEventInfo(EventType et, SipCallInfo::pointer callInfo)
		: m_et(et), m_callInfo(callInfo)
		, m_eventData(-1)
	{}
protected:
	EventType m_et;
	SipCallInfo::pointer m_callInfo;

	int m_eventData;					// For Register
};

class OnSipHandler
{
public:
	virtual void onSipEvent(SipEventInfo::pointer eventInfo) = 0;
};

class DoSipHandler
{
public:
	typedef boost::shared_ptr<DoSipHandler> pointer;

	virtual bool initSip(const CSipParameter & sipp, OnSipHandler * handler) = 0;
	virtual bool isInitSip(void) const = 0;
	virtual void quitSip(void) = 0;

	virtual const CSipParameter & getSipParameter(void) const = 0;
	virtual SipCallInfo::pointer getSipCallInfo(void) const = 0;

	virtual int sipRegister(void) = 0;
	virtual void sipUnRegister(void) = 0;
	virtual bool isSipRegistered(void) const = 0;

	// call control
	virtual int sipCallInvite(const tstring & callee_num) = 0; ///呼叫
	virtual int sipCallAnswer(SipCallInfo::pointer callInfo, int localaudioport, int localvideoport) = 0;       ///应答呼叫
	virtual int sipCallAnswer(SipCallInfo::pointer callInfo) = 0;       ///应答呼叫
	virtual int sipCallTerminate(SipCallInfo::pointer callInfo) = 0;                    ///挂机
	virtual int sipCallSendDtmf(SipCallInfo::pointer callInfo, char dtmf) = 0;

};

#endif // __libsiphandler_h__
