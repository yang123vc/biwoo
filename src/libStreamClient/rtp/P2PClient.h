/*
    StreamModule is a P2P and conference communication module base on CGCP.
    Copyright (C) 2009-2010  Akee Yang <akee.yang@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// P2PClient.h file here
#ifndef __P2PClient_h__
#define __P2PClient_h__

//
// include
#include <boost/shared_ptr.hpp>
#include "../libstreamhandler.h"

// 
class P2PClient
	: public CgcClientHandler
	, public CDoP2PClientHandler
{
public:
	static CDoP2PClientHandler::pointer create(DoSotpClientHandler::pointer pDoHandler)
	{
		return CDoP2PClientHandler::pointer(new P2PClient(pDoHandler));
	}

public:
	void clearP2PHandler(void);
	bool isStarted(void) const {return m_doHaneler.get() != NULL;}

private:
	// CgcClientHandler handler
	virtual void OnCgcResponse(const cgcParser & response);
	virtual void OnCgcResponse(CCgcData::pointer recvData);

	////////////////////////////////////////
	// CDoP2PClientHandler handler
	virtual void setP2PHandler(CP2PHandler * handler, bool disableSotpParse);

	virtual const tstring & getP2PUser(void) const {return m_sp2PUser;}
	virtual long getP2PType(void) const {return m_nP2PType;}
	virtual void setP2PParam(long newV) {m_nP2PParam = newV;}
	virtual long getP2PParam(void) const {return m_nP2PParam;}

	virtual void setP2PAddress(const tstring & newV) {m_sP2PAddress = newV;}
	virtual const tstring & getP2PAddress(void) const {return m_sP2PAddress;}

	virtual void setP2PInfo(const tstring & sP2PUser, long nP2PType);

	virtual void clearP2PStatus(void);
	virtual void setLocalP2PStatus(bool newv) {m_bLocalP2PStatus = newv;}
	virtual bool getLocalP2PStatus(void) const {return m_bLocalP2PStatus;}
	virtual void setRemoteP2PStatus(bool newv) {m_bRemoteP2PStatus = newv;}
	virtual bool getRemoteP2PStatus(void) const {return m_bRemoteP2PStatus;}
	virtual DoSotpClientHandler::pointer dohandler(void) const {return m_doHaneler;}

	//virtual void doSetMediaType(unsigned short mediaType) {if (m_doHaneler != NULL) m_doHaneler->doSetMediaType(mediaType);}
	//virtual size_t doSendP2PData(const unsigned char * data, size_t size, unsigned long timestamp) {return m_doHaneler == NULL ? 0 : m_doHaneler->doSendData(data, size, timestamp);}

	///////// forward to DoSotpClientHandler
	// response handler
	virtual void doSetResponseHandler(CgcClientHandler * newv) {m_doHaneler->doSetResponseHandler(newv);}
	virtual const CgcClientHandler * doGetResponseHandler(void) const {return m_doHaneler->doGetResponseHandler();}
	virtual void doSetDisableSotpParser(bool newv) {m_doHaneler->doSetDisableSotpParser(newv);}

	// session 
	virtual bool doSendOpenSession(ULONG * pOutCallId = 0) {return m_doHaneler->doSendOpenSession(pOutCallId);}
	virtual void doSendCloseSession(ULONG * pOutCallId = 0) {m_doHaneler->doSendCloseSession(pOutCallId);}
	virtual bool doIsSessionOpened(void) const {return m_doHaneler->doIsSessionOpened();}
	virtual const tstring & doGetSessionId(void) const {return m_doHaneler->doGetSessionId();}

	// app call
	virtual void doBeginCallLock(void) {m_doHaneler->doBeginCallLock();}
	virtual bool doSendAppCall(ULONG nCallSign, const tstring & sCallName,
		cgcAttachment::pointer pAttach = constNullAttchment, ULONG * pOutCallId = 0) {return m_doHaneler->doSendAppCall(nCallSign, sCallName, pAttach, pOutCallId);}

	// thread
	virtual void doSetCIDTResends(UINT timeoutResends=2, UINT timeoutSeconds=4) {m_doHaneler->doSetCIDTResends(timeoutResends, timeoutSeconds);}
	virtual void doStartRecvThreads(unsigned short nRecvThreads = 2) {m_doHaneler->doStartRecvThreads(nRecvThreads);}
	virtual void doStartActiveThread(unsigned int nActiveWaitSeconds = 60) {m_doHaneler->doStartActiveThread(nActiveWaitSeconds);}

	// parameter
	virtual void doAddParameter(cgcParameter::pointer parameter) {m_doHaneler->doAddParameter(parameter);}
	virtual void doAddParameters(const cgcParameterList & parameters) {m_doHaneler->doAddParameters(parameters);}
	virtual ULONG doGetParameterSize(void) const {return m_doHaneler->doGetParameterSize();}

	// info
	virtual void doSetEncoding(const tstring & newv=_T("GBK")) {m_doHaneler->doSetEncoding(newv);}
	virtual const tstring & doGetEncoding(void) const {return m_doHaneler->doGetEncoding();}
	virtual void doSetAppName(const tstring & newv) {m_doHaneler->doSetAppName(newv);}
	virtual const tstring & doGetAppName(void) const {return m_doHaneler->doGetAppName();}
	virtual void doSetAccount(const tstring & account, const tstring & passwd) {m_doHaneler->doSetAccount(account, passwd);}
	virtual void doGetAccount(tstring & account, tstring & passwd) const {m_doHaneler->doGetAccount(account, passwd);}
	virtual const tstring & doGetClientType(void) const {return m_doHaneler->doGetClientType();}

	// other
	virtual void doSetRemoteAddr(const tstring & newv) {m_doHaneler->doSetRemoteAddr(newv);}
	virtual void doSetMediaType(unsigned short newv) {m_doHaneler->doSetMediaType(newv);}	// for RTP
	virtual size_t doSendData(const unsigned char * data, size_t size) {return m_doHaneler->doSendData(data, size);}
	virtual size_t doSendData(const unsigned char * data, size_t size, unsigned int timestamp) {return m_doHaneler->doSendData(data, size, timestamp);}	// for RTP

public:
	P2PClient(DoSotpClientHandler::pointer pDoHandler);
	virtual ~P2PClient(void);

private:
	tstring m_sp2PUser;
	long m_nP2PType;
	long m_nP2PParam;
	tstring m_sP2PAddress;	// IP:PORT
	bool	m_bLocalP2PStatus;	// 本地是否已经打通P2P
	bool	m_bRemoteP2PStatus;	// 远端是否已经打通P2P

	DoSotpClientHandler::pointer m_doHaneler;
	CP2PHandler * m_pP2PHandler;
};

//typedef std::map<tstring, P2PClient*>	P2PClientMap;
//typedef std::pair<tstring, P2PClient*>	P2PClientPair;
//typedef P2PClientMap::const_iterator		P2PClientMapCIter;
//typedef P2PClientMap::iterator			P2PClientMapIter;


#endif // __P2PClient_h__
