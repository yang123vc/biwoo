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

#define libP2P_EXPORTS

#include "P2PProxy.h"
#include "ConnectManager.h"

//////////////////////////////////////////////////////
//
ConnectManager m_ConnManager;

CP2PProxy::CP2PProxy(void)
{

}

CP2PProxy::~CP2PProxy(void)
{

}


bool CP2PProxy::connectStreamServer(const CCgcAddress & ServerAddr, const CCgcAddress & RtpAddr, const CCgcAddress & UdpAddr)
{
	if (m_ConnManager.isConnectedStreamServer())
		return true;

	return m_ConnManager.connectStreamServer(ServerAddr, RtpAddr, UdpAddr);
}


void CP2PProxy::disconnectStreamServer(void)
{
	m_ConnManager.disconnectStreamServer();
}

bool CP2PProxy::isConnectedStreamServer(void)
{
	return m_ConnManager.isConnectedStreamServer();
}

void CP2PProxy::SetP2PAVHandler(OnP2PHandler * handler)
{
	m_ConnManager.setP2PAVHandler(handler);
}

bool CP2PProxy::AccountLogin(const tstring & sAccountID)
{
	if (!m_ConnManager.isConnectedStreamServer())
		return false;

	return m_ConnManager.Login(sAccountID);
}

void CP2PProxy::AccountLogout(void)
{
	m_ConnManager.Logout();
}

bool CP2PProxy::IsAccountLogined(void)
{
	return m_ConnManager.IsLogined();
}

bool CP2PProxy::AVP2PConnect(const tstring & sFriendID, P2PAVConnectType p2pConnectType)
{
	return m_ConnManager.requestAVP2P(sFriendID, p2pConnectType);
}

void CP2PProxy::AVP2PDisconnect(const tstring & sFriendID, P2PAVConnectType p2pConnectType)
{
	m_ConnManager.disconnectAVP2P(sFriendID, p2pConnectType);
}

long CP2PProxy::SendFile(const tstring & sFriendID, const tstring & filepath, const tstring & filename)
{
	return m_ConnManager.SendFile(sFriendID, filepath, filename);
}

bool CP2PProxy::AcceptFile(long fid, const tstring & savetofilepath, const tstring & filename)
{
	return m_ConnManager.AcceptFile(fid, savetofilepath, filename);
}

bool CP2PProxy::RejectFile(long fid)
{
	return m_ConnManager.RejectFile(fid);
}

bool CP2PProxy::CancelFile(long fid)
{
	return m_ConnManager.CancelFile(fid);
}

DoDSHandler::pointer CP2PProxy::OpenLocalAV(const CAVParameter & avp)
{
	return m_ConnManager.startServer(avp);
}

void CP2PProxy::CloseLocalAV(void)
{
	m_ConnManager.stopServer();
}

DoDSHandler::pointer CP2PProxy::OpenRemoteAV(const CAVParameter & avp)
{
	return m_ConnManager.startClient(avp);
}

void CP2PProxy::RemoteVideoReversal(void)
{
	m_ConnManager.clientVideoReversal();
}

void CP2PProxy::CloseRemoteAV(DoDSHandler::pointer pDoDSHandler)
{
	m_ConnManager.stopClient(pDoDSHandler);
}
