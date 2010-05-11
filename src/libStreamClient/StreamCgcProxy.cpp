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

#include "StreamCgcProxy.h"
#pragma warning(disable:4819)

//CStreamCgcProxy::CStreamCgcProxy(CStreamHandler * handler)
CStreamCgcProxy::CStreamCgcProxy(void)
: m_handler(NULL)
, m_bDoAccountUnRegister(false)
, m_cgcClient(NULL)

{
}

CStreamCgcProxy::~CStreamCgcProxy(void)
{
	avsStop();
}

bool CStreamCgcProxy::avsStart(const CCgcAddress & serverAddr, const CCgcAddress & rtpAddr, const CCgcAddress & udpAddr)
{
	m_serverAddr = serverAddr;
	m_rtpAddr = rtpAddr;
	m_udpAddr = udpAddr;

	if (m_cgcClient == NULL)
		m_cgcClient = m_sotpClient.startClient(m_serverAddr);

	if (m_cgcClient == NULL)
		return false;

	m_cgcClient->doStartActiveThread();
	m_cgcClient->doSetResponseHandler(this);
	m_cgcClient->doSetAppName(const_Avs_AppName);
	bool ret = m_cgcClient->doSendOpenSession();
	if (ret)
	{
		int i=0;
		while (i++ < 20)
		{
			if (m_cgcClient->doIsSessionOpened())
				return true;
#ifdef WIN32
			Sleep(100);
#else
			usleep(100000);
#endif
		}
	}

	return ret;
}

void CStreamCgcProxy::avsStop(void)
{
	if (m_cgcClient != NULL)
	{
		avsAccountUnRegister();
#ifdef WIN32
		Sleep(200);
#else
		usleep(200000);
#endif
		DoSotpClientHandler * handlertemp = m_cgcClient;
		m_cgcClient = NULL;

		handlertemp->doSendCloseSession();
		m_sotpClient.stopClient(handlertemp);
	}
}

bool CStreamCgcProxy::avsAccountRegister(const tstring & sUsername, const tstring & sPassword)
{
	if (!avsIsOpenSession()) return false;
	if (!m_sCurrentUser.empty()) return true;

	m_bDoAccountUnRegister = false;

	m_cgcClient->doBeginCallLock();
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("UserName"), sUsername));
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("Password"), sPassword));
	m_cgcClient->doSendAppCall(const_CallSign_AccountRegister, const_Avs_Api_AccountRegister);

	int i=0;
	while (i++ < 20)
	{
		if (!m_sCurrentUser.empty())
			return true;
#ifdef WIN32
		Sleep(100);
#else
		usleep(100000);
#endif
	}

	return true;
}

bool CStreamCgcProxy::avsAccountUnRegister(void)
{
	if (!avsIsOpenSession()) return false;
	if (m_sCurrentUser.empty()) return false;

	m_bDoAccountUnRegister = true;
	boost::mutex::scoped_lock lock(m_p2pProxy.mutex());
	CLockMap<tstring, CDoP2PClientHandler::pointer>::iterator pIter;
	for (pIter=m_p2pProxy.begin(); pIter!=m_p2pProxy.end(); pIter++)
	{
		CDoP2PClientHandler::pointer pP2PClient = pIter->second;
		if (pP2PClient->getLocalP2PStatus() && pP2PClient->getRemoteP2PStatus())
			avsDisconnectP2PUser(pP2PClient);

		DoSotpClientHandler * pDoHandler = pP2PClient->dohandler();
		BOOST_ASSERT(pDoHandler != NULL);

		pDoHandler->doSetRemoteAddr(pP2PClient->getP2PType() == CDoP2PClientHandler::P2P_AUDIO
			|| pP2PClient->getP2PType() == CDoP2PClientHandler::P2P_VIDEO ? m_rtpAddr.address() : m_udpAddr.address());
		m_sotpClient.stopClient(pDoHandler);
	}
	m_p2pProxy.clear(false);


	m_cgcClient->doBeginCallLock();
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("UserName"), m_sCurrentUser));
	m_cgcClient->doSendAppCall(const_CallSign_AccountUnRegister, const_Avs_Api_AccountUnRegister);

	m_sCurrentUser = _T("");
	return true;
}

bool CStreamCgcProxy::createConference(const tstring & conferenceName)
{
	if (!avsIsOpenSession()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("Conference"), conferenceName));

	m_cgcClient->doSendAppCall(const_CallSign_CreateConference, const_Avs_Api_CreateConference);
	return true;
}

bool CStreamCgcProxy::enableAudioSend(const tstring & conferenceName, int memberIndex, bool enable)
{
	if (!avsIsOpenSession()) return false;
	//if (m_sCurrentUser.empty()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("Conference"), conferenceName));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Index"), memberIndex));
	m_cgcClient->doAddParameter(cgcParameter::create2(_T("Enable"), enable));

	m_cgcClient->doSendAppCall(const_CallSign_EnableAudioSend, const_Avs_Api_EnableAudioSend);
	return true;
}

bool CStreamCgcProxy::enableAudioRecv(const tstring & conferenceName, int memberIndex, bool enable)
{
	if (!avsIsOpenSession()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("Conference"), conferenceName));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Index"), memberIndex));
	m_cgcClient->doAddParameter(cgcParameter::create2(_T("Enable"), enable));

	m_cgcClient->doSendAppCall(const_CallSign_EnableAudioRecv, const_Avs_Api_EnableAudioRecv);
	return true;
}

bool CStreamCgcProxy::enableVideoSend(const tstring & conferenceName, int memberIndex, bool enable)
{
	if (!avsIsOpenSession()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("Conference"), conferenceName));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Index"), memberIndex));
	m_cgcClient->doAddParameter(cgcParameter::create2(_T("Enable"), enable));

	m_cgcClient->doSendAppCall(const_CallSign_EnableVideoSend, const_Avs_Api_EnableVideoSend);
	return true;
}

bool CStreamCgcProxy::enableVideoRecv(const tstring & conferenceName, int memberIndex, bool enable)
{
	if (!avsIsOpenSession()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("Conference"), conferenceName));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Index"), memberIndex));
	m_cgcClient->doAddParameter(cgcParameter::create2(_T("Enable"), enable));

	m_cgcClient->doSendAppCall(const_CallSign_EnableVideoRecv, const_Avs_Api_EnableVideoRecv);
	return true;
}

bool CStreamCgcProxy::selectVideoRecv(const tstring & conferenceName, int memberIndex, int selectIndex)
{
	if (!avsIsOpenSession()) return false;
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Conference"), conferenceName));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("SrcIndex"), memberIndex));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("SelectIndex"), selectIndex));

	m_cgcClient->doSendAppCall(const_CallSign_SelectVideoRecv, const_Avs_Api_SelectVideoRecv);
	return true;
}

bool CStreamCgcProxy::selectVideoRecvByIndentify(const tstring & conferenceName, const tstring & memberIndentify, int selectIndex)
{
	if (!avsIsOpenSession()) return false;
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Conference"), conferenceName));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("MemberIndentify"), memberIndentify));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("SelectIndex"), selectIndex));

	m_cgcClient->doSendAppCall(const_CallSign_SelectVideoRecvByIndentify, const_Avs_Api_SelectVideoRecvByIndentify);
	return true;
}


int CStreamCgcProxy::avsRequestP2PUser(const tstring & sRequestUser, long nP2PType, long nP2PParam)
{
	if (!avsIsOpenSession()) return -1;
	if (m_sCurrentUser.empty()) return -1;

	char buffer[30];
	sprintf(buffer, ":%d-%d", nP2PType, nP2PParam);
	tstring sP2PKey(sRequestUser);
	sP2PKey.append(buffer);
	CDoP2PClientHandler::pointer pP2PClient;
	if (!m_p2pProxy.find(sP2PKey, pP2PClient, false))
	{
		DoSotpClientHandler * pDoHandler = m_sotpClient.startClient(nP2PType == CDoP2PClientHandler::P2P_AUDIO || nP2PType == CDoP2PClientHandler::P2P_VIDEO ? m_rtpAddr : m_udpAddr);
		if (pDoHandler == NULL) return -1;
		pDoHandler->doSetAppName(const_Avs_AppName);

		pP2PClient = P2PClient::create(pDoHandler);
		m_p2pProxy.insert(sP2PKey, pP2PClient);
	}else if (pP2PClient->getLocalP2PStatus() && pP2PClient->getRemoteP2PStatus())
	{
		return 1;	// is already P2P state
	}

	pP2PClient->setP2PInfo(sRequestUser, nP2PType);
	pP2PClient->clearP2PStatus();

	DoSotpClientHandler * pDoHandler = pP2PClient->dohandler();
	BOOST_ASSERT(pDoHandler != NULL);

	pDoHandler->doSetResponseHandler(this);
	pDoHandler->doSetRemoteAddr(nP2PType == CDoP2PClientHandler::P2P_AUDIO || nP2PType == CDoP2PClientHandler::P2P_VIDEO
		? m_rtpAddr.address() : m_udpAddr.address());
	pDoHandler->doSetMediaType(const_CGCMediaType);

	if (nP2PType == CDoP2PClientHandler::P2P_AUDIO || nP2PType == CDoP2PClientHandler::P2P_VIDEO)
	{
		for (int i=0; i<=2; i++)
		{
			pDoHandler->doBeginCallLock();
			pDoHandler->doSendAppCall(0, const_Avs_Api_RequestP2PUser);
		}
	}

	pDoHandler->doSetDisableSotpParser(false);
	pDoHandler->doBeginCallLock();
	pDoHandler->doAddParameter(cgcParameter::create(_T("FromUser"), m_sCurrentUser));
	pDoHandler->doAddParameter(cgcParameter::create(_T("ToUser"), sRequestUser));
	pDoHandler->doAddParameter(cgcParameter::create(_T("P2PType"), (long)nP2PType));
	pDoHandler->doAddParameter(cgcParameter::create(_T("P2PParam"), nP2PParam));
	pDoHandler->doSendAppCall(const_CallSign_RequestP2PUser, const_Avs_Api_RequestP2PUser);

	return 0;
}

bool CStreamCgcProxy::avsResponseP2PUser(const tstring & sResponseUser, long nP2PType, long nP2PParam)
{
	if (!avsIsOpenSession()) return false;
	if (m_sCurrentUser.empty()) return false;

	char buffer[30];
	sprintf(buffer, ":%d-%d", nP2PType, nP2PParam);
	tstring sP2PKey(sResponseUser);
	sP2PKey.append(buffer);
	CDoP2PClientHandler::pointer pP2PClient;
	if (!m_p2pProxy.find(sP2PKey, pP2PClient, false))
	{
		DoSotpClientHandler * pDoHandler = m_sotpClient.startClient(nP2PType == CDoP2PClientHandler::P2P_AUDIO || nP2PType == CDoP2PClientHandler::P2P_VIDEO ? m_rtpAddr : m_udpAddr);
		if (pDoHandler == NULL) return false;
		pDoHandler->doSetAppName(const_Avs_AppName);

		pP2PClient = P2PClient::create(pDoHandler);
		m_p2pProxy.insert(sP2PKey, pP2PClient);
	}
	pP2PClient->setP2PInfo(sResponseUser, nP2PType);
	pP2PClient->clearP2PStatus();


	DoSotpClientHandler * pDoHandler = pP2PClient->dohandler();
	BOOST_ASSERT(pDoHandler != NULL);

	pDoHandler->doSetResponseHandler(this);
	pDoHandler->doSetRemoteAddr(nP2PType == CDoP2PClientHandler::P2P_AUDIO || nP2PType == CDoP2PClientHandler::P2P_VIDEO 
		? m_rtpAddr.address() : m_udpAddr.address());
	pDoHandler->doSetMediaType(const_CGCMediaType);

	if (nP2PType == CDoP2PClientHandler::P2P_AUDIO || nP2PType == CDoP2PClientHandler::P2P_VIDEO)
	{
		for (int i=0; i<=2; i++)
		{
			pDoHandler->doBeginCallLock();
			pDoHandler->doSendAppCall(0, const_Avs_Api_ResponseP2PUser);
		}
	}

	pDoHandler->doSetDisableSotpParser(false);
	pDoHandler->doBeginCallLock();
	pDoHandler->doAddParameter(cgcParameter::create(_T("FromUser"), m_sCurrentUser));
	pDoHandler->doAddParameter(cgcParameter::create(_T("ToUser"), sResponseUser));
	pDoHandler->doAddParameter(cgcParameter::create(_T("P2PType"), (long)nP2PType));
	pDoHandler->doAddParameter(cgcParameter::create(_T("P2PParam"), nP2PParam));
	pDoHandler->doSendAppCall(const_CallSign_ResponseP2PUser, const_Avs_Api_ResponseP2PUser);

	return true;
}

int CStreamCgcProxy::avsDisconnectP2PUser(const tstring & sP2PUser, long nP2PType, long nP2PParam)
{
	if (!avsIsOpenSession()) return -1;
	if (m_sCurrentUser.empty()) return -1;

	char buffer[30];
	sprintf(buffer, ":%d-%d", nP2PType, nP2PParam);
	tstring sP2PKey(sP2PUser);
	sP2PKey.append(buffer);
	CDoP2PClientHandler::pointer pP2PClient;
	if (!m_p2pProxy.find(sP2PKey, pP2PClient, false))
	{
		// Already disconnect
		return 1;
	}

	m_cgcClient->doBeginCallLock();
	m_cgcClient->doAddParameter(cgcParameter::create(_T("FromUser"), m_sCurrentUser));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("ToUser"), sP2PUser));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("P2PType"), (long)nP2PType));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("P2PParam"), (long)nP2PParam));
	m_cgcClient->doSendAppCall(const_CallSign_DisconnectP2PUser, const_Avs_Api_DisconnectP2PUser);

	return 0;
}

bool CStreamCgcProxy::avsDisconnectP2PUser(CDoP2PClientHandler::pointer p2pClient)
{
	BOOST_ASSERT (p2pClient.get() != NULL);

	if (!avsIsOpenSession()) return false;
	if (m_sCurrentUser.empty()) return false;

	m_cgcClient->doBeginCallLock();
	m_cgcClient->doAddParameter(cgcParameter::create(_T("FromUser"), m_sCurrentUser));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("ToUser"), p2pClient->getP2PUser()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("P2PType"), p2pClient->getP2PType()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("P2PParam"), p2pClient->getP2PParam()));
	m_cgcClient->doSendAppCall(const_CallSign_DisconnectP2PUser, const_Avs_Api_DisconnectP2PUser);

	return true;
}

CDoP2PClientHandler::pointer CStreamCgcProxy::getP2PClient(const tstring & sP2PUser, long nP2PType, long nP2PParam) const
{
	char buffer[30];
	sprintf(buffer, ":%d-%d", nP2PType, nP2PParam);
	tstring sP2PKey(sP2PUser);
	sP2PKey.append(buffer);
	CDoP2PClientHandler::pointer result;
	m_p2pProxy.find(sP2PKey, result);
	return result;
}

bool CStreamCgcProxy::sendP2PAck(CDoP2PClientHandler::pointer pP2PClient, long nP2PType, long nP2PParam, bool bAckAck)
{
	BOOST_ASSERT (pP2PClient.get() != NULL);

	if (!avsIsOpenSession()) return false;
	if (m_sCurrentUser.empty()) return false;
	if (pP2PClient == NULL) return false;

	DoSotpClientHandler * pDoHandler = pP2PClient->dohandler();
	BOOST_ASSERT(pDoHandler != NULL);

	pDoHandler->doBeginCallLock();
	if (bAckAck)
		pDoHandler->doAddParameter(cgcParameter::create(_T("P2PACKACK"), _T("1")));
	pDoHandler->doAddParameter(cgcParameter::create(_T("FromUser"), m_sCurrentUser));
	pDoHandler->doAddParameter(cgcParameter::create(_T("P2PType"), nP2PType));
	pDoHandler->doAddParameter(cgcParameter::create(_T("P2PParam"), nP2PParam));
	pDoHandler->doSendAppCall(const_CallSign_P2PAck, const_Avs_Api_P2PAck);
	return true;
}

bool CStreamCgcProxy::avsGetAllUser(void)
{
	if (!avsIsOpenSession()) return false;
	if (m_sCurrentUser.empty()) return false;

	m_cgcClient->doBeginCallLock();
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FromUser"), m_sCurrentUser));
	m_cgcClient->doSendAppCall(const_CallSign_GetAllUser, const_Avs_Api_GetAllUser);

	return true;
}
/*
bool CStreamCgcProxy::avsSendTextMessage(const tstring & sFriendName, const tstring & sMsg)
{
	if (!avsIsOpenSession()) return false;
	if (m_sCurrentUser.empty()) return false;

	if (m_sCurrentUser.empty() || sMsg.empty()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FromUser"), m_sCurrentUser));
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("ToUser"), sFriendName));
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("Msg"), sMsg));

	m_cgcClient->doSendAppCall(const_CallSign_SendTextMessage, const_Avs_Api_SendTextMessage);
	return true;
}

bool CStreamCgcProxy::avsSendTextMessageP2P(const tstring & sFriendName, const tstring & sMsg)
{
	if (!avsIsOpenSession()) return false;
	if (m_sCurrentUser.empty()) return false;

	// ? im-text:
	tstring sP2PKey(sFriendName);
	sP2PKey.append(_T(":im-text"));
	CDoP2PClientHandler::pointer pP2PClient;
	if (!m_p2pProxy.find(sP2PKey, pP2PClient, false))
		return false;

	if (!pP2PClient->getLocalP2PStatus() || !pP2PClient->getRemoteP2PStatus())
		return false;

	DoSotpClientHandler * pDoHandler = pP2PClient->dohandler();
	BOOST_ASSERT(pDoHandler != NULL);

#ifdef _UNICODE
	std::string sMsgTemp = cgcString::W2Char(sMsg);
	pDoHandler->doSendData((const unsigned char*)sMsgTemp.c_str(), sMsgTemp.length());
#else
	pDoHandler->doSendData((const unsigned char*)sMsg.c_str(), sMsg.size());
#endif // _UNICODE
	return true;
}

bool CStreamCgcProxy::avsSendP2PData(const tstring & sFriendName, const tstring & sP2PType, const unsigned char * pData, unsigned int nSize)
{
	if (!avsIsOpenSession()) return false;
	if (sFriendName.empty() || m_sCurrentUser.empty()) return false;
	if (pData == NULL || nSize == 0) return false;

	tstring sP2PKey(sFriendName);
	sP2PKey.append(_T(":"));
	sP2PKey.append(sP2PType);
	CDoP2PClientHandler::pointer pP2PClient;
	if (!m_p2pProxy.find(sP2PKey, pP2PClient, false))
		return false;

	if (!pP2PClient->getLocalP2PStatus() || !pP2PClient->getRemoteP2PStatus())
		return false;

	DoSotpClientHandler * pDoHandler = pP2PClient->dohandler();
	BOOST_ASSERT(pDoHandler != NULL);

	pDoHandler->doSendData(pData, nSize);
	return true;
}

bool CStreamCgcProxy::avsSendFileTransfer(const tstring & sFriendName, const tstring & sFileName)
{
	if (!avsIsOpenSession()) return false;
	if (m_sCurrentUser.empty()) return false;
	if (sFileName.empty()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FromUser"), m_sCurrentUser));
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("ToUser"), sFriendName));
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("TransferType"), _T("FILE")));
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("TransferName"), sFileName));

	m_cgcClient->doSendAppCall(const_CallSign_RequestTransfer, const_Avs_Api_RequestTransfer);
	return true;
}

bool CStreamCgcProxy::avsResponseTransfer(const tstring & sFriendName, const tstring & sTransferType)
{
	if (!avsIsOpenSession()) return false;
	if (m_sCurrentUser.empty()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FromUser"), m_sCurrentUser));
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("ToUser"), sFriendName));
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("TransferType"), sTransferType));

	m_cgcClient->doSendAppCall(const_CallSign_ResponseTransfer, const_Avs_Api_ResponseTransfer);
	return true;
}

bool CStreamCgcProxy::avsDownFile(const tstring & sFileName)
{
	if (!avsIsOpenSession()) return false;
	if (m_sCurrentUser.empty()) return false;
	if (sFileName.empty()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FromUser"), m_sCurrentUser));
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FileName"), sFileName));
//	m_cgcClient.setCIDTResends(2, 4);

	m_cgcClient->doSendAppCall(const_CallSign_DownFile, const_Avs_Api_DownFile);
	return true;
}

bool CStreamCgcProxy::avsUpFile(const tstring & sFileName)
{
	if (!avsIsOpenSession()) return false;
	if (m_sCurrentUser.empty()) return false;
	if (sFileName.empty()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FromUser"), m_sCurrentUser));
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FileName"), sFileName));
//	m_cgcClient.setCIDTResends(2, 4);

	m_cgcClient->doSendAppCall(const_CallSign_UpFile, const_Avs_Api_UpFile);
	return true;
}

bool CStreamCgcProxy::avsTransferFile(const tstring & sFriendName, const tstring & sFileName)
{
	if (!avsIsOpenSession()) return false;
	if (m_sCurrentUser.empty()) return false;
	if (sFileName.empty()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FromUser"), m_sCurrentUser));
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("ToUser"), sFriendName));
	m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FileName"), sFileName));
//	m_cgcClient.setCIDTResends(2, 4);

//	m_cgcClient->doSendAppCall(const_CallSign_TransferFile, const_Avs_Api_TransferFile);
//	m_cgcClient->doSendAppCall(const_CallSign_DownFile, const_Avs_Api_DownFile);
	return true;
}
*/

static std::fstream fsClientPath;
ULONG nTotal = 0;

void CStreamCgcProxy::OnCgcResponse(const cgcParser & response)
{
	if (response.isResulted() && response.isOpenType())
	{
		return;
	}
	if (m_handler == NULL || m_cgcClient == NULL) return;

	long resultValue = response.getResultValue();
	switch (response.getSign())
	{
	case 1001:
		{
			// Text Message
			const tstring & sFromUser = response.getRecvParameterValue(_T("FromUser"));
			const tstring & sMsg = response.getRecvParameterValue(_T("Msg"));

			//m_handler->onSendTextMessage(sFromUser, sMsg);
		}break;
	case 1002:
		{
			// From TCP/IP
			// P2P Request Message, From Server
			const tstring & sFromUser = response.getRecvParameterValue(_T("FromUser"));
			const tstring & sRemoteAddr = response.getRecvParameterValue(_T("RemoteAddr"));
			long nP2PType = response.getRecvParameterValue(_T("P2PType"), 0);
			long nP2PParam = response.getRecvParameterValue(_T("P2PParam"), 0);

			char buffer[30];
			sprintf(buffer, ":%d-%d", nP2PType, nP2PParam);
			tstring sP2PKey(sFromUser);
			sP2PKey.append(buffer);
			CDoP2PClientHandler::pointer pP2PClient;
			if (!m_p2pProxy.find(sP2PKey, pP2PClient, false))
			{
				DoSotpClientHandler * pDoHandler = m_sotpClient.startClient(nP2PType == CDoP2PClientHandler::P2P_AUDIO || nP2PType == CDoP2PClientHandler::P2P_VIDEO
					? m_rtpAddr : m_udpAddr);
				if (pDoHandler == NULL) break;
				pDoHandler->doSetAppName(const_Avs_AppName);

				pP2PClient = P2PClient::create(pDoHandler);
				m_p2pProxy.insert(sP2PKey, pP2PClient);
			}
			pP2PClient->setP2PInfo(sFromUser, nP2PType);
			pP2PClient->setP2PAddress(sRemoteAddr);
			pP2PClient->setP2PParam(nP2PParam);

			// Response P2P Request, To Server
			avsResponseP2PUser(sFromUser, nP2PType, nP2PParam);
		}break;
	case 1003:
		{
			// From TCP/IP
			// P2P Response Message, From Server
			const tstring & sFromUser = response.getRecvParameterValue(_T("FromUser"));
			const tstring & sRemoteAddr = response.getRecvParameterValue(_T("RemoteAddr"));
			long nP2PType = response.getRecvParameterValue(_T("P2PType"), 0);
			long nP2PParam = response.getRecvParameterValue(_T("P2PParam"), 0);

			char buffer[30];
			sprintf(buffer, ":%d-%d", nP2PType, nP2PParam);
			tstring sP2PKey(sFromUser);
			sP2PKey.append(buffer);
			CDoP2PClientHandler::pointer pP2PClient;
			if (!m_p2pProxy.find(sP2PKey, pP2PClient, false))
				break;

			pP2PClient->setP2PInfo(sFromUser, nP2PType);
			pP2PClient->setP2PAddress(sRemoteAddr);
			pP2PClient->setP2PParam(nP2PParam);

			// 1003: P2P ACK
			DoSotpClientHandler * pDoHandler = pP2PClient->dohandler();
			BOOST_ASSERT(pDoHandler != NULL);

			pDoHandler->doSetRemoteAddr(sRemoteAddr);
			for (int i=0; i<=2; i++)
				pDoHandler->doSendData((const unsigned char*)"p2ptest", 7);
			for (int i=0; i<20; i++)
			{
				if (m_bDoAccountUnRegister)
					break;

				if (pP2PClient->getLocalP2PStatus())
					sendP2PAck(pP2PClient, nP2PType, nP2PParam, true);
				else
					sendP2PAck(pP2PClient, nP2PType, nP2PParam, false);
#ifdef WIN32
				Sleep(500);
#else
				usleep(500000);
#endif

				if (pP2PClient->getLocalP2PStatus() && pP2PClient->getRemoteP2PStatus())
					break;
			}

			//CDoP2PClientHandler * pClientHandler = (CDoP2PClientHandler*)pP2PClient.get();
			if (pP2PClient->getLocalP2PStatus() && pP2PClient->getRemoteP2PStatus())
			{
				bool disableSotpParse = true;
				CP2PHandler * pP2PHandler = m_handler->onP2PUserAck(pP2PClient, disableSotpParse);
				if (pP2PHandler != NULL)
					pP2PClient->setP2PHandler(pP2PHandler, disableSotpParse);

			}else
			{
				m_handler->onP2PRequestTimeout(pP2PClient);
			}
		}break;
	case 1004:
		{
			// From TCP/IP
			// P2P Response Message return From Server
			const tstring & sToUser = response.getRecvParameterValue(_T("ToUser"));
			long nP2PType = response.getRecvParameterValue(_T("P2PType"), 0);
			long nP2PParam = response.getRecvParameterValue(_T("P2PParam"), 0);

			char buffer[30];
			sprintf(buffer, ":%d-%d", nP2PType, nP2PParam);
			tstring sP2PKey(sToUser);
			sP2PKey.append(buffer);
			CDoP2PClientHandler::pointer pP2PClient;
			if (!m_p2pProxy.find(sP2PKey, pP2PClient, false))
				break;

			DoSotpClientHandler * pDoHandler = pP2PClient->dohandler();
			BOOST_ASSERT(pDoHandler != NULL);

			pDoHandler->doSetRemoteAddr(pP2PClient->getP2PAddress());
	
			// 1003: P2P ACK
			for (int i=0; i<=2; i++)
				pDoHandler->doSendData((const unsigned char*)"p2ptest", 7);
			for (int i=0; i<20; i++)
			{
				if (m_bDoAccountUnRegister)
					break;

				if (pP2PClient->getLocalP2PStatus())
					sendP2PAck(pP2PClient, nP2PType, nP2PParam, true);
				else
					sendP2PAck(pP2PClient, nP2PType, nP2PParam, false);
#ifdef WIN32
				Sleep(500);
#else
				usleep(500000);
#endif
				if (pP2PClient->getLocalP2PStatus() && pP2PClient->getRemoteP2PStatus())
					break;
			}

			if (pP2PClient->getLocalP2PStatus() && pP2PClient->getRemoteP2PStatus())
			{
				bool disableSotpParse = true;
				//CDoP2PClientHandler * pClientHandler = (CDoP2PClientHandler*)pP2PClient.get();
				CP2PHandler * pP2PHandler = m_handler->onP2PUserAck(pP2PClient, disableSotpParse);
				if (pP2PHandler != NULL)
					pP2PClient->setP2PHandler(pP2PHandler, disableSotpParse);

			}

		}break;
	case 1005:
		{
			// From TCP/IP
			// P2P Disconnect Message, From Server
			const tstring & sFromUser = response.getRecvParameterValue(_T("FromUser"));
			long nP2PType = response.getRecvParameterValue(_T("P2PType"), 0);
			long nP2PParam = response.getRecvParameterValue(_T("P2PParam"), 0);

			char buffer[30];
			sprintf(buffer, ":%d-%d", nP2PType, nP2PParam);
			tstring sP2PKey(sFromUser);
			sP2PKey.append(buffer);
			CDoP2PClientHandler::pointer pP2PClient;
			if (m_p2pProxy.find(sP2PKey, pP2PClient, true))
			{
				//CDoP2PClientHandler * pClientHandler = (CDoP2PClientHandler*)pP2PClient.get();
				pP2PClient->clearP2PStatus();
				m_handler->onP2PUserDisconnect(pP2PClient);

				DoSotpClientHandler * pDoHandler = pP2PClient->dohandler();
				BOOST_ASSERT(pDoHandler != NULL);
				pDoHandler->doSetRemoteAddr(nP2PType == CDoP2PClientHandler::P2P_AUDIO || nP2PType == CDoP2PClientHandler::P2P_VIDEO ? m_rtpAddr.address() : m_udpAddr.address());
				m_sotpClient.stopClient(pDoHandler);
			}
		}break;
	case 1006:
		{
			const tstring & sFromUser = response.getRecvParameterValue(_T("FromUser"));
			long nP2PType = response.getRecvParameterValue(_T("P2PType"), 0);
			long nP2PParam = response.getRecvParameterValue(_T("P2PParam"), 0);


			m_handler->onP2PRequestResult(sFromUser, nP2PType, resultValue);
			if (resultValue != 0)
			{
				char buffer[30];
				sprintf(buffer, ":%d-%d", nP2PType, nP2PType);
				tstring sP2PKey(sFromUser);
				sP2PKey.append(buffer);
				CDoP2PClientHandler::pointer pP2PClient;
				if (m_p2pProxy.find(sP2PKey, pP2PClient, true))
				{
					DoSotpClientHandler * pDoHandler = pP2PClient->dohandler();
					BOOST_ASSERT(pDoHandler != NULL);
					pDoHandler->doSetRemoteAddr(nP2PType == CDoP2PClientHandler::P2P_AUDIO || nP2PType == CDoP2PClientHandler::P2P_VIDEO
						? m_rtpAddr.address() : m_udpAddr.address());
					m_sotpClient.stopClient(pDoHandler);
				}
			}
		}break;
	case 2001:
		{
			// User login notify
			const tstring & sFromUser = response.getRecvParameterValue(_T("FromUser"));
			m_handler->onUserLogined(sFromUser);
		}break;
	case 2002:
		{
			// User login notify
			const tstring & sFromUser = response.getRecvParameterValue(_T("FromUser"));
			m_handler->onUserLogouted(sFromUser);
		}break;
/*	case 3001:
		{
			// 3001: Transfer Request Message, From Server
			const tstring & sCfid = response.getRecvParameterValue(_T("cfid"));
			const tstring & sFromUser = response.getRecvParameterValue(_T("FromUser"));
			const tstring & sTransferType = response.getRecvParameterValue(_T("TransferType"));
			const tstring & sTransferName = response.getRecvParameterValue(_T("TransferName"));
			const tstring & sRemoteAddr = response.getRecvParameterValue(_T("RemoteAddr"));


			tstring sIp = cgcString::formatStr(sRemoteAddr, _T(""), _T(":"));
			tstring sPort = cgcString::formatStr(sRemoteAddr, _T(":"), _T(""));
			unsigned short nPort = (unsigned short)_ttoi(sPort.c_str());
		}break;
	case 3002:
		{
			// 3002: Transfer Response Message, From Server
			const tstring & sFromUser = response.getRecvParameterValue(_T("FromUser"));
			const tstring & sTransferType = response.getRecvParameterValue(_T("TransferType"));
			const tstring & sTransferName = response.getRecvParameterValue(_T("TransferName"));

			std::fstream fsClientPath;
			fsClientPath.open("d:/myvideoM16.avi", std::ios::out);
			if (fsClientPath.is_open())
			{
				//
				char buffer[1024];
				memset(buffer, 0, 1024);
				fsClientPath.read(buffer, 1024);
				//std::streambuf * rdBuffer = fsClientPath.rdbuf();
				//rdBuffer->g

				//m_transfClient.SendData((const unsigned char*)buffer, 1024, 1);

				fsClientPath.close();
			}
		}break;
	case const_CallSign_UpFile:
		{
			// 开始传附件
			const tstring & sFileName = response.getRecvParameterValue(_T("FileName"));
			const UINT nOneSize = 1024*3;
			char buffer[nOneSize+1];
			memset(buffer, 0, nOneSize+1);
			if (resultValue == 1)
			{
				if (fsClientPath.is_open())
					fsClientPath.close();
				fsClientPath.open("d:/myvideoM16.avi", std::ios::in|std::ios::binary);
				if (fsClientPath.is_open())
				{
					fsClientPath.seekp(0, std::ios::end);
					nTotal = fsClientPath.tellp();
					fsClientPath.seekp(0, std::ios::beg);
					//
					fsClientPath.read(buffer, nOneSize);

					cgc::cgcAttachment::pointer attach(cgcAttachment::create());
					if (fsClientPath.eof())
						attach->setAttach((const unsigned char*)buffer, nTotal);
					else
						attach->setAttach((const unsigned char*)buffer, nOneSize);
					attach->setName("file");
					attach->setTotal(nTotal);
					attach->setIndex(0);
					m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FileName"), sFileName));
					m_cgcClient->doSendAppCall(const_CallSign_UpFile, const_Avs_Api_UpFile, attach);
				}
			}else if (resultValue == 0)
			{
				cgcParameter::pointer pIndex = response.getRecvParameter(_T("INDEX"));

				ULONG nIndex = 0;
				if (pIndex.get() != NULL)
				{
					nIndex = cgcString::toULongValue(pIndex->getValue().c_str(), 0);
				}

				if (!fsClientPath.is_open())
					break;

				if (nIndex + nOneSize >= nTotal)
					break;

				nIndex += nOneSize;
				fsClientPath.seekp(nIndex);
				fsClientPath.read(buffer, nOneSize);

				cgc::cgcAttachment::pointer attach(cgcAttachment::create());
				if (fsClientPath.eof())
				{
					attach->setAttach((const unsigned char*)buffer, nTotal-nIndex);
					fsClientPath.close();
				}else
					attach->setAttach((const unsigned char*)buffer, nOneSize);
				if (attach->getAttachSize() == 0)
				{
					fsClientPath.close();
					break;
				}
				attach->setName("file");
				attach->setTotal(nTotal);
				attach->setIndex(nIndex);
				m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FileName"), sFileName));
				m_cgcClient->doSendAppCall(const_CallSign_UpFile, const_Avs_Api_UpFile, attach);
			}
		}break;
	case const_CallSign_DownFile:
		{
			if (resultValue == 1)
			{
				fsClientPath.close();
			}
			const tstring & sFilename = response.getRecvParameterValue(_T("FileName"));
			if (response.isRecvHasAttachInfo())
			{
				cgcAttachment::pointer attach = response.getRecvAttachment();

				if (attach.get() == NULL || attach->getIndex() == 0)
				{
					fsClientPath.open("d:/myvideoM16-2.avi", std::ios::out|std::ios::binary);
				}

				if (fsClientPath.is_open())
				{
					fsClientPath.seekp(attach->getIndex());
					fsClientPath.write((const char*)attach->getAttachData(), attach->getAttachSize());
					fsClientPath.flush();
				}

				if (attach->getIndex() + attach->getAttachSize() >= attach->getTotal())
				{
					fsClientPath.close();
				}

				//CString sTemp = _T("");
				//sTemp.Format(_T("%d"), attach->getIndex());
				char sTemp[10];
				memset(sTemp, 0, sizeof(sTemp));
				sprintf(sTemp, _T("%d"), attach->getIndex());
				m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FileName"), sFilename));
				m_cgcClient->doAddParameter(cgcParameter::create(cgcParameter::PT_INT, _T("INDEX"), sTemp));
				m_cgcClient->doSendAppCall(const_CallSign_DownFile, const_Avs_Api_DownFile);
			}
		}break;
	case const_CallSign_TransferFile:
		{
			// 开始传附件
			const UINT nOneSize = 1024*2;
			if (resultValue == 1)
			{
				if (fsClientPath.is_open())
					fsClientPath.close();
				fsClientPath.open("d:/myvideoM16.avi", std::ios::in|std::ios::binary);
				if (fsClientPath.is_open())
				{
					fsClientPath.seekp(0, std::ios::end);
					std::fstream::pos_type posTotal = fsClientPath.tellp();
					fsClientPath.seekp(0, std::ios::beg);
					//
					char buffer[nOneSize+1];
					memset(buffer, 0, nOneSize+1);
					fsClientPath.read(buffer, nOneSize);

					cgc::cgcAttachment::pointer attach(cgcAttachment::create());
					if (fsClientPath.eof())
						attach->setAttach((const unsigned char*)buffer, posTotal);
					else
						attach->setAttach((const unsigned char*)buffer, nOneSize);
					attach->setName("file");
					attach->setTotal(posTotal);
					attach->setIndex(0);
					m_cgcClient->doSendAppCall(const_CallSign_TransferFile, const_Avs_Api_TransferFile, attach);
				}
			}else if (resultValue == 0)
			{
				cgcParameter::pointer pTotal = response.getRecvParameter(_T("TOTAL"));
				cgcParameter::pointer pIndex = response.getRecvParameter(_T("INDEX"));

//				ULONG nTotal = 0;
				ULONG nIndex = 0;
				if (pTotal.get() != NULL && pIndex.get() != NULL)
				{
					if (nTotal == 0)
						nTotal = cgcString::toULongValue(pTotal->getValue().c_str(), 0);
					nIndex = cgcString::toULongValue(pIndex->getValue().c_str(), 0);
				}

				if (!fsClientPath.is_open())
					break;

				if (nIndex + nOneSize >= nTotal)
					break;

				char buffer[nOneSize+1];
				memset(buffer, 0, nOneSize+1);
				nIndex += nOneSize;
				fsClientPath.seekp(nIndex);
				fsClientPath.read(buffer, nOneSize);

				cgc::cgcAttachment::pointer attach(cgcAttachment::create());
				if (fsClientPath.eof())
				{
					attach->setAttach((const unsigned char*)buffer, nTotal-nIndex);
					fsClientPath.close();
				}else
					attach->setAttach((const unsigned char*)buffer, nOneSize);
				if (attach->getAttachSize() == 0)
				{
					fsClientPath.close();
					break;
				}
				attach->setName("file");
				attach->setTotal(nTotal);
				attach->setIndex(nIndex);
				m_cgcClient->doSendAppCall(const_CallSign_TransferFile, const_Avs_Api_TransferFile, attach);
			}

		}break;
	case const_CallSign_RequestTransfer:
		{
			const tstring & sCfid = response.getRecvParameterValue(_T("cfid"));
			const tstring & sRemoteAddr = response.getRecvParameterValue(_T("RemoteAddr"));

			tstring sIp = cgcString::formatStr(sRemoteAddr, _T(""), _T(":"));
			tstring sPort = cgcString::formatStr(sRemoteAddr, _T(":"), _T(""));
			unsigned short nPort = (unsigned short)_ttoi(sPort.c_str());

//			m_transfClient.CleaerDest();
//			m_transfClient.AddDest(sIp.c_str(), nPort);
//			//std::string sCommConferCmd = "ss:join";
//			//m_transfClient.SendData((const unsigned char*)sCommConferCmd.c_str(), sCommConferCmd.length(), 2);
//			char pBuffer[100];
//			memset(pBuffer, 0, sizeof(pBuffer));
//			sprintf(pBuffer,
//				"JOIN \n"
//				"Cfid: %s\n",
//#ifdef _UNICODE
//				cgcString::W2Char(sCfid).c_str());
//#else
//				sCfid.c_str());
//#endif // _UNICODE
//			m_transfClient.SendData((const unsigned char*)pBuffer, strlen(pBuffer), 1);

		}break;
	case const_CallSign_ResponseTransfer:
		break;*/
	case const_CallSign_AccountRegister:
		{
			// 登录成功
			if (resultValue == 0)
			{
				cgcParameter::pointer pUserName = response.getRecvParameter(_T("UserName"));
				if (pUserName.get() != NULL)
				{
					this->m_sCurrentUser = pUserName->getValue();
					m_handler->onUserLogined(m_sCurrentUser);
				}
			}
			return;
		}break;
	case const_CallSign_AccountUnRegister:
		{
			m_handler->onUserLogouted(m_sCurrentUser);
			m_sCurrentUser = _T("");
		}break;
	case const_CallSign_GetAllUser:
		{
			cgcParameter::pointer pUserId = response.getRecvParameter(_T("UserId"));
			cgcParameter::pointer pUserName = response.getRecvParameter(_T("FromUser"));
			if (pUserId.get() != NULL && pUserName.get() != NULL)
			{
				m_handler->onUserInfo(pUserId->getValue(), pUserName->getValue());
			}

		}break;
	case const_CallSign_RequestP2PUser:
	case const_CallSign_ResponseP2PUser:
	case const_CallSign_SendTextMessage:
		break;
	case const_CallSign_DisconnectP2PUser:
		{
			if (m_bDoAccountUnRegister)
				break;
			// From TCP/IP
			// P2P Disconnect Message, From Server
			const tstring & sFromUser = response.getRecvParameterValue(_T("FromUser"));
			long nP2PType = response.getRecvParameterValue(_T("P2PType"), 0);
			long nP2PParam = response.getRecvParameterValue(_T("P2PParam"), 0);

			char buffer[30];
			sprintf(buffer, ":%d-%d", nP2PType, nP2PParam);
			tstring sP2PKey(sFromUser);
			sP2PKey.append(buffer);
			CDoP2PClientHandler::pointer pP2PClient;
			if (m_p2pProxy.find(sP2PKey, pP2PClient, true))
			{
				//CDoP2PClientHandler * pClientHandler = (CDoP2PClientHandler*)pP2PClient.get();
				pP2PClient->clearP2PStatus();
				m_handler->onP2PUserDisconnect(pP2PClient);

				DoSotpClientHandler * pDoHandler = pP2PClient->dohandler();
				BOOST_ASSERT(pDoHandler != NULL);
				pDoHandler->doSetRemoteAddr(nP2PType == CDoP2PClientHandler::P2P_AUDIO || nP2PType == CDoP2PClientHandler::P2P_VIDEO
					? m_rtpAddr.address() : m_udpAddr.address());
				m_sotpClient.stopClient(pDoHandler);
			}
		}break;
	case const_CallSign_P2PAck:
		{
			// P2P ACK, From P2P user
			const tstring & sP2PAckAck = response.getRecvParameterValue(_T("P2PACKACK"));
			const tstring & sFromUser = response.getRecvParameterValue(_T("FromUser"));
			long nP2PType = response.getRecvParameterValue(_T("P2PType"), 0);
			long nP2PParam = response.getRecvParameterValue(_T("P2PParam"), 0);

			char buffer[30];
			sprintf(buffer, ":%d-%d", nP2PType, nP2PParam);
			tstring sP2PKey(sFromUser);
			sP2PKey.append(buffer);
			CDoP2PClientHandler::pointer pP2PClient;
			if (!m_p2pProxy.find(sP2PKey, pP2PClient, false))
				break;

			if (sP2PAckAck.compare(_T("1")) == 0)
			{
				pP2PClient->setRemoteP2PStatus(true);
			}else if (!sFromUser.empty())
			{
				if (pP2PClient->getLocalP2PStatus())
				{
					DoSotpClientHandler * pDoHandler = pP2PClient->dohandler();
					BOOST_ASSERT(pDoHandler != NULL);

					//pDoHandler->doSetRemoteAddr(destIp, destPort);
					//pDoHandler->doSetRemoteAddr(pP2PClient->getP2PAddress());
					for (int i=1; i<=2; i++)
						pDoHandler->doSendData((const unsigned char*)"p2ptest", 7);
					sendP2PAck(pP2PClient, nP2PType, nP2PParam, false);	// 对方请求打通本地状态
				}else
				{
					pP2PClient->setLocalP2PStatus(true);
					sendP2PAck(pP2PClient, nP2PType, nP2PParam, true);
				}
			}
		}break;
	default:
		{
		}break;
	}

}


//void CStreamCgcProxy::onReceiveEvent(const BYTE* pFramedata, int nFrameLen, u_long destIp, u_int destPort)
//{
//	// ？？ 不能新建文件，要调试中转机制！
//	std::string sTransferName = "d:\\recv_abc.avi";
//	std::fstream fsClientPath;
//	fsClientPath.open(sTransferName.c_str(), std::ios::in);
//	if (fsClientPath.is_open())
//	{
//		fsClientPath.write((const char*)pFramedata, nFrameLen);
//		fsClientPath.close();
//	}
//}
