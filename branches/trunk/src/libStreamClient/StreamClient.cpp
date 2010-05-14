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


#define libAVSCLIENT_EXPORTS

#include "StreamClient.h"
#include "StreamCgcProxy.h"

////////////////////////////////////////////////
//
CStreamCgcProxy gAvsProxy;

CStreamClient::CStreamClient(void)

{
}

CStreamClient::~CStreamClient(void)
{

}

bool CStreamClient::avsStart(const CCgcAddress & serverAddr, const CCgcAddress & rtpAddr, const CCgcAddress & udpAddr, CStreamHandler * pHandler)
{
	gAvsProxy.setAvsHandler(pHandler);
	return gAvsProxy.avsStart(serverAddr, rtpAddr, udpAddr);
}

void CStreamClient::avsStop(void)
{
	gAvsProxy.setAvsHandler(NULL);
	gAvsProxy.avsStop();
}

bool CStreamClient::avsIsStarted(void) const
{
	return gAvsProxy.avsIsStarted();
}

bool CStreamClient::avsIsOpenSession(void) const
{
	return gAvsProxy.avsIsOpenSession();
}

bool CStreamClient::avsAccountRegister(const tstring & sUsername, const tstring & sPassword)
{
	return gAvsProxy.avsAccountRegister(sUsername, sPassword);
}
bool CStreamClient::avsAccountUnRegister(void)
{
	return gAvsProxy.avsAccountUnRegister();
}

const tstring & CStreamClient::getCurrentUser(void) const
{
	return gAvsProxy.getCurrentUser().c_str();
}

bool CStreamClient::avsGetAllUser(void)
{
	return gAvsProxy.avsGetAllUser();
}

int CStreamClient::avsRequestP2PUser(const tstring & sRequestUser, long nP2PType, long nP2PParam)
{
	return gAvsProxy.avsRequestP2PUser(sRequestUser, nP2PType, nP2PParam);
}

int CStreamClient::avsDisconnectP2PUser(const tstring & sP2PUser, long nP2PType, long nP2PParam)
{
	return gAvsProxy.avsDisconnectP2PUser(sP2PUser, nP2PType, nP2PParam);
}

bool CStreamClient::avsDisconnectP2PUser(CDoP2PClientHandler::pointer p2pClient)
{
	return gAvsProxy.avsDisconnectP2PUser(p2pClient);
}

CDoP2PClientHandler::pointer CStreamClient::getP2PClient(const tstring & sP2PUser, long nP2PType, long nP2PParam) const
{
	return gAvsProxy.getP2PClient(sP2PUser, nP2PType, nP2PParam);
}

bool CStreamClient::createConference(const tstring & conferenceName)
{
	return gAvsProxy.createConference(conferenceName);
}

bool CStreamClient::enableAudioSend(const tstring & conferenceName, int memberIndex, bool enable)
{
	return gAvsProxy.enableAudioSend(conferenceName, memberIndex, enable);
}

bool CStreamClient::enableAudioRecv(const tstring & conferenceName, int memberIndex, bool enable)
{
	return gAvsProxy.enableAudioRecv(conferenceName, memberIndex, enable);
}

bool CStreamClient::enableVideoSend(const tstring & conferenceName, int memberIndex, bool enable)
{
	return gAvsProxy.enableVideoSend(conferenceName, memberIndex, enable);
}

bool CStreamClient::enableVideoRecv(const tstring & conferenceName, int memberIndex, bool enable)
{
	return gAvsProxy.enableVideoRecv(conferenceName, memberIndex, enable);
}

bool CStreamClient::selectVideoRecv(const tstring & conferenceName, int memberIndex, int selectIndex)
{
	return gAvsProxy.selectVideoRecv(conferenceName, memberIndex, selectIndex);
}
bool CStreamClient::selectVideoRecvByIndentify(const tstring & conferenceName, const tstring & memberIndentify, int selectIndex)
{
	return gAvsProxy.selectVideoRecvByIndentify(conferenceName, memberIndentify, selectIndex);
}