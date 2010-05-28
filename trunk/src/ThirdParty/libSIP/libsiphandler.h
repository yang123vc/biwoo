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
		// ע��״̬: 1 ��ʼ 2 �ɹ� 3 ʧ�� 4 ��ֹ
		ET_Register = 0x01

		// �յ�����
		, ET_CallInvite = 0x21

		// ���ڳ��Ժ��жԷ�...
		, ET_CallProceeding = 0x22

		// �Է�����
		, ET_CallRinging = 0x23

		// ����ʧ��: �Է�æ���߾ܾ�
		, ET_CallRequestFailure = 0x24

		// �Է�ժ��
		, ET_CallAnswered = 0x25

		// ������Ϣ����DTMF��
		, ET_CallMessageNew = 0x31

		// ���н������һ���
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
	// ע���¼�:
	// 1 ��ʼ 2 �ɹ� 3 ʧ�� 4 ��ֹ
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
	virtual int sipCallInvite(const tstring & callee_num) = 0; ///����
	virtual int sipCallAnswer(SipCallInfo::pointer callInfo, int localaudioport, int localvideoport) = 0;       ///Ӧ�����
	virtual int sipCallAnswer(SipCallInfo::pointer callInfo) = 0;       ///Ӧ�����
	virtual int sipCallTerminate(SipCallInfo::pointer callInfo) = 0;                    ///�һ�
	virtual int sipCallSendDtmf(SipCallInfo::pointer callInfo, char dtmf) = 0;

};

#endif // __libsiphandler_h__
