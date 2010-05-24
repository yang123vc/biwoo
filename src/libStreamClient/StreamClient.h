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

// StreamClient.h file here
#pragma once
#ifndef __StreamClient_h__
#define __StreamClient_h__

#include "dlldefine.h"
#include "libstreamhandler.h"

class LIBAVSCLIENT_CLASS CStreamClient
{
public:
	bool avsStart(const CCgcAddress & serverAddr, CStreamHandler * pHandler);
	void avsStop(void);
	bool avsIsStarted(void) const;
	bool avsIsOpenSession(void) const;

	// account manager
	bool avsAccountRegister(const tstring & sUsername, const tstring & sPassword);
	bool avsAccountUnRegister(void);
	const tstring & getCurrentUser(void) const;
	bool avsGetAllUser(void);	// ?

	// P2P
	int avsRequestP2PUser(const tstring & sRequestUser, long nP2PType, long nP2PParam = 0);
	int avsDisconnectP2PUser(const tstring & sP2PUser, long nP2PType, long nP2PParam);
	bool avsDisconnectP2PUser(CDoP2PClientHandler::pointer p2pClient);

	CDoP2PClientHandler::pointer getP2PClient(const tstring & sP2PUser, long nP2PType, long nP2PParam) const;

	/////////////////////////////////
	// Conference
	bool createConference(const tstring & conferenceName);
	// memberIndex: 0 ALL
	bool enableAudioSend(const tstring & conferenceName, int memberIndex, bool enable);
	// memberIndex: 0 ALL
	bool enableAudioRecv(const tstring & conferenceName, int memberIndex, bool enable);
	// memberIndex: 0 FALSE
	bool enableVideoSend(const tstring & conferenceName, int memberIndex, bool enable);
	// memberIndex: 0 ALL
	bool enableVideoRecv(const tstring & conferenceName, int memberIndex, bool enable);

	bool selectVideoRecv(const tstring & conferenceName, int memberIndex, int selectIndex); //add by xap
	bool selectVideoRecvByIndentify(const tstring & conferenceName, const tstring & memberIndentify, int selectIndex); //add by xap

	// message
	//bool avsSendTextMessageP2P(const tstring & sFriendName, const tstring & sMsg);	// P2PType=im-text

public:
	CStreamClient(void);
	~CStreamClient(void);
};

#endif // __StreamClient_h__
