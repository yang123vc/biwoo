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

#include "P2PClient.h"

P2PClient::P2PClient(DoSotpClientHandler::pointer pDoHandler)
: m_sp2PUser(_T(""))
, m_nP2PType(0)
, m_nP2PParam(0)
, m_sP2PAddress(_T(""))
, m_bLocalP2PStatus(false)
, m_bRemoteP2PStatus(false)
, m_doHaneler(pDoHandler)
, m_pP2PHandler(NULL)

{
	BOOST_ASSERT(m_doHaneler.get() != NULL);
}

P2PClient::~P2PClient(void)
{
}

void P2PClient::setP2PHandler(CP2PHandler * handler, bool disableSotpParse)
{
	BOOST_ASSERT (handler != NULL);

	m_pP2PHandler = handler;
	if (m_doHaneler != NULL)
	{
		m_doHaneler->doSetDisableSotpParser(disableSotpParse);
		m_doHaneler->doSetResponseHandler(this);
	}
}

void P2PClient::clearP2PHandler(void)
{
	m_pP2PHandler = NULL;
}

void P2PClient::setP2PInfo(const tstring & sP2PUser, long nP2PType)
{
	m_sp2PUser = sP2PUser;
	m_nP2PType = nP2PType;
}

void P2PClient::clearP2PStatus(void)
{
	m_bLocalP2PStatus = false;
	m_bRemoteP2PStatus = false;
	m_pP2PHandler = NULL;
}

void P2PClient::OnCgcResponse(const cgcParserSotp & response)
{
	if (m_pP2PHandler != NULL)
		//m_pP2PHandler->onP2PEvent((DoP2PClientHandler*)this, response);
		m_pP2PHandler->onP2PEvent(shared_from_this(), response);
		
}

void P2PClient::OnCgcResponse(CCgcData::pointer recvData)
{
	if (m_pP2PHandler != NULL)
		//m_pP2PHandler->onP2PEvent((DoP2PClientHandler*)this, recvData);
		m_pP2PHandler->onP2PEvent(shared_from_this(), recvData);
}

//size_t P2PClient::sendP2PData(const unsigned char * pData, int nLen, LONGLONG nSeqNumber)
//{
////	if (!m_bRemoteP2PStatus) return 0;
//	return m_rtpClient.SendData(pData, nLen, nSeqNumber);
//}

//void P2PClient::onReceiveEvent(CRTPData::pointer receiveData)
//{
//	if (m_pP2PHandler)
//	{
//		// 改用P2PClient重新定义事件
//		// m_pOwnerHandler->onReceiveEvent(buffer, recvSize, destIp, destPort);
//		m_pP2PHandler->onP2pRecvEvent(*this, receiveData);
//		return;
//	}
//
//	CgcRtpClient::onReceiveEvent(receiveData);
//}

