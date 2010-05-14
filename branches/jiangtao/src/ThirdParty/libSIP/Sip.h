// Sip.h file here
#ifndef __Sip_h__
#define __Sip_h__

#include "dlldefine.h"
#include "libsiphandler.h"
#include "SipParameter.h"

class LIBSIP_CLASS CSip
{
public:
	bool initSip(const CSipParameter & sipp, OnSipHandler * handler);
	void quitSip(void);
	bool isInitSip(void) const;
	const CSipParameter & GetSipParameter(void) const;

	int sipRegister(void);
	void sipUnRegister(void);

	// call control
	int CallInvite(const tstring & callee_num);
	int CallAnswer(SipCallInfo::pointer callInfo, int localaudioport, int localvideoport);
	int CallTerminate(SipCallInfo::pointer callInfo);
	int CallSendDtmf(SipCallInfo::pointer callInfo, char dtmf);

public:
	CSip(void);
	~CSip(void);

};

#endif // __Sip_h__
