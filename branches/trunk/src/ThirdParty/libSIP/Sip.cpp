
#include "Sip.h"
#include "Siptransmit.h"

Siptransmit gSiptransmit;

CSip::CSip(void)
{

}

CSip::~CSip(void)
{

}

bool CSip::initSip(const CSipParameter & sipp, OnSipHandler * handler)
{
	BOOST_ASSERT (handler != NULL);

	return gSiptransmit.initSip(sipp, handler);
}

void CSip::quitSip(void)
{
	gSiptransmit.quitSip();
}

bool CSip::isInitSip(void) const
{
	return gSiptransmit.isInitSip();
}

const CSipParameter & CSip::GetSipParameter(void) const
{
	return gSiptransmit.GetSipParameter();
}

int CSip::sipRegister(void)
{
	return gSiptransmit.sipRegister();
}

void CSip::sipUnRegister(void)
{
	gSiptransmit.sipUnRegister();
}

// call control
int CSip::CallInvite(const tstring & callee_num)
{
	return gSiptransmit.CallInvite(callee_num);
}

int CSip::CallAnswer(SipCallInfo::pointer callInfo, int localaudioport, int localvideoport)
{
	return gSiptransmit.CallAnswer(callInfo, localaudioport, localvideoport);
}

int CSip::CallTerminate(SipCallInfo::pointer callInfo)
{
	return gSiptransmit.CallTerminate(callInfo);
}

int CSip::CallSendDtmf(SipCallInfo::pointer callInfo, char dtmf)
{
	return gSiptransmit.CallSendDtmf(callInfo, dtmf);
}
