
#include "Rtp.h"
#include "RtpProxy.h"
#include "../stl/lockmap.h"

CLockMap<DoRtpHandler*, CRtpProxy::pointer> m_rtpProxy;

CRtp::CRtp(void)
{
}

CRtp::~CRtp(void)
{
	//CLockMap<DoRtpHandler*, CRtpProxy::pointer>::const_iterator iter;
	//for (iter=m_rtpProxy.begin(); iter!=m_rtpProxy.end(); iter++)
	//{
	//	DoRtpHandler * handler = (DoRtpHandler*)iter->second.get();
	//	handler->doSetRtpHandler(0);
	//	iter->second->DestroySession();
	//}
	//m_rtpProxy.clear();
}

DoRtpHandler * CRtp::startRtp(unsigned short rtpPort)
{
	CRtpProxy::pointer rtpProxy = CRtpProxy::create();
	CRtpProxy::InitComm();
	if (rtpProxy->InitSession(rtpPort) != 0)
		return NULL;

	//rtpProxy->AddDest(sIp.c_str(), nPort);
	//rtpProxy->SetHandle(this);

	//rtpProxy->
	DoRtpHandler * handler = (DoRtpHandler*)rtpProxy.get();
	m_rtpProxy.insert(handler, rtpProxy);
	return handler;
}

void CRtp::stopRtp(DoRtpHandler * handler)
{
	CRtpProxy::pointer rtpProxy;
	if (m_rtpProxy.find(handler, rtpProxy, true))
	{
		//handler->doSetRtpHandler(0);
		rtpProxy->DestroySession();
		handler->doSetRtpHandler(0);
	}
}
