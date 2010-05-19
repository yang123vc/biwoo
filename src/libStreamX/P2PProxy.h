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

// P2PProxy.h file here
#ifndef __P2PProxy_h__
#define __P2PProxy_h__

#include <stl/stldef.h>
#include <CGCLib/cgcaddress.h>
#include <libDS/AVParameter.h>
#include <libDS/libdshandler.h>
#include "libP2PAVDef.h"
#include "dlldefine.h"
#include "libstreamxhandler.h"

const CCgcAddress defaultCgcAddress;

class LIBP2PAV_CLASS CP2PProxy
{
public:
	bool connectStreamServer(const CCgcAddress & ServerAddr, const CCgcAddress & RtpAddr, const CCgcAddress & UdpAddr = defaultCgcAddress);
	void disconnectStreamServer(void);
	bool isConnectedStreamServer(void);

	void SetP2PAVHandler(OnP2PHandler * handler);
	bool AccountLogin(const tstring & sAccountID);
	void AccountLogout(void);
	bool IsAccountLogined(void);

	bool AVP2PConnect(const tstring & sFriendID, P2PAVConnectType p2pConnectType = P2PAVConnect::CT_Both);
	void AVP2PDisconnect(const tstring & sFriendID, P2PAVConnectType p2pConnectType = P2PAVConnect::CT_Both);

	long SendFile(const tstring & sFriendID, const tstring & filepath, const tstring & filename);
	bool AcceptFile(long fid, const tstring & savetofilepath, const tstring & filename);
	bool RejectFile(long fid);
	bool CancelFile(long fid);

	DoDSHandler::pointer OpenLocalAV(const CAVParameter & avp);
	void CloseLocalAV(void);

	DoDSHandler::pointer OpenRemoteAV(const CAVParameter & avp);
	void RemoteVideoReversal(void);
	void CloseRemoteAV(DoDSHandler::pointer pDoDSHandler);

public:
	CP2PProxy(void);
	~CP2PProxy(void);

};

#endif // __P2PProxy_h__
