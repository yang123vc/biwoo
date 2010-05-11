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

// StreamCgcProxy.h file here
#pragma once
#ifndef __StreamCgcProxy_h__
#define __StreamCgcProxy_h__

//
// 添加头文件
// 
//#include "../CGCLib/cgclibinclude.h"
#include <CGCLib/CGCLib.h>
#include <CGCBase/cgcString.h>

//#include "rtp/CgcP2PClient.h"
#include "rtp/P2PClient.h"
#include "libstreamhandler.h"


////////////////////////////
// const 
const unsigned short const_CGCMediaType	= 1;

const tstring const_Avs_AppName					= _T("StreamModule");
// account manager
const tstring const_Avs_Api_AccountRegister		= _T("AccountRegister");
const ULONG const_CallSign_AccountRegister		= 0x0001;
const tstring const_Avs_Api_AccountUnRegister	= _T("AccountUnRegister");
const ULONG const_CallSign_AccountUnRegister	= 0x0002;
// P2P manager
const tstring const_Avs_Api_RequestP2PUser		= _T("RequestP2PUser");
const ULONG const_CallSign_RequestP2PUser		= 0x0003;
const tstring const_Avs_Api_ResponseP2PUser		= _T("ResponseP2PUser");
const ULONG const_CallSign_ResponseP2PUser		= 0x0004;
const tstring const_Avs_Api_P2PAck				= _T("P2PAck");
const ULONG const_CallSign_P2PAck				= 0x0005;
const tstring const_Avs_Api_DisconnectP2PUser	= _T("DisconnectP2PUser");
const ULONG const_CallSign_DisconnectP2PUser	= 0x0006;

// Transfer manager
const tstring const_Avs_Api_RequestTransfer		= _T("RequestTransfer");
const ULONG const_CallSign_RequestTransfer		= 0x0011;
const tstring const_Avs_Api_ResponseTransfer	= _T("ResponseTransfer");
const ULONG const_CallSign_ResponseTransfer		= 0x0012;
// Transfer File
const tstring const_Avs_Api_TransferFile		= _T("TransferFile");
const ULONG const_CallSign_TransferFile			= 0x0013;
const tstring const_Avs_Api_DownFile			= _T("DownFile");
const ULONG const_CallSign_DownFile				= 0x0014;
const tstring const_Avs_Api_UpFile				= _T("UpFile");
const ULONG const_CallSign_UpFile				= 0x0015;

// Conference
const tstring const_Avs_Api_CreateConference	= _T("CreateConference");
const ULONG const_CallSign_CreateConference		= 0x0021;
const tstring const_Avs_Api_EnableAudioSend		= _T("EnableAudioSend");
const ULONG const_CallSign_EnableAudioSend		= 0x0022;
const tstring const_Avs_Api_EnableAudioRecv		= _T("EnableAudioRecv");
const ULONG const_CallSign_EnableAudioRecv		= 0x0023;
const tstring const_Avs_Api_EnableVideoSend		= _T("EnableVideoSend");
const ULONG const_CallSign_EnableVideoSend		= 0x0024;
const tstring const_Avs_Api_EnableVideoRecv		= _T("EnableVideoRecv");
const ULONG const_CallSign_EnableVideoRecv		= 0x0025;

const tstring const_Avs_Api_SelectVideoRecv		= _T("SelectVideoRecv"); //add by xap
const ULONG const_CallSign_SelectVideoRecv		= 0x0026; //add by xap

const tstring const_Avs_Api_SelectVideoRecvByIndentify		= _T("SelectVideoRecvByIndentify"); //add by xap
const ULONG const_CallSign_SelectVideoRecvByIndentify		= 0x0027; //add by xap

//const tstring const_Avs_Api_ResponseTransfer	= L"ResponseTransfer";
//const ULONG const_CallSign_ResponseTransfer		= 0x0012;

// ???
const tstring const_Avs_Api_GetAllUser			= _T("GetAllUser");		// ?
const ULONG const_CallSign_GetAllUser			= 0x0111;

const tstring const_Avs_Api_SendTextMessage		= _T("SendTextMessage");	// ?
const ULONG const_CallSign_SendTextMessage		= 0x0121;


///////////////////////////////
// handler

class CStreamCgcProxy
	: public CgcClientHandler
	//, public OnRtpHandler
{
private:
	CStreamHandler * m_handler;
	CSotpClient m_sotpClient;
	DoSotpClientHandler * m_cgcClient;

	bool m_bDoAccountUnRegister;
	tstring m_sCurrentUser;
	CCgcAddress m_serverAddr;
	CCgcAddress m_rtpAddr;					// for P2P
	CCgcAddress m_udpAddr;					// for P2P

	CLockMap<tstring, CDoP2PClientHandler::pointer> m_p2pProxy;

	//CRtpProxy m_transfClient;	// ?

public:
	//const CgcBaseClient & getCgcClient(void) const {return m_cgcClient;}
	void setAvsHandler(CStreamHandler * newv) {m_handler = newv;}

	bool avsStart(const CCgcAddress & serverAddr, const CCgcAddress & rtpAddr, const CCgcAddress & udpAddr);
	void avsStop(void);
	bool avsIsStarted(void) const {return m_cgcClient != NULL;}
	bool avsIsOpenSession(void) const {return m_cgcClient == NULL ? false : m_cgcClient->doIsSessionOpened();}

	// account manager
	bool avsAccountRegister(const tstring & sUsername, const tstring & sPassword);
	bool avsAccountUnRegister(void);
	const tstring & getCurrentUser(void) const {return m_sCurrentUser;}

	// Conference
	bool createConference(const tstring & conferenceName);
	bool enableAudioSend(const tstring & conferenceName, int memberIndex, bool enable);
	bool enableAudioRecv(const tstring & conferenceName, int memberIndex, bool enable);
	bool enableVideoSend(const tstring & conferenceName, int memberIndex, bool enable);
	bool enableVideoRecv(const tstring & conferenceName, int memberIndex, bool enable);

	bool selectVideoRecv(const tstring & conferenceName, int memberIndex, int selectIndex); //add by xap
	bool selectVideoRecvByIndentify(const tstring & conferenceName, const tstring & memberIndentify, int selectIndex); //add by xap

	// P2P manager
	// 0: Request OK
	// 1: isalready P2P state
	// -1: Request Error
	int avsRequestP2PUser(const tstring & sRequestUser, long nP2PType, long nP2PParam = 0);
	// 0: Request OK
	// 1: isalready disconnect state
	// -1: Request Error
	int avsDisconnectP2PUser(const tstring & sP2PUser, long nP2PType, long nP2PParam);
	bool avsDisconnectP2PUser(CDoP2PClientHandler::pointer p2pClient);
	CDoP2PClientHandler::pointer getP2PClient(const tstring & sP2PUser, long nP2PType, long nP2PParam) const;

	bool avsGetAllUser(void);	// ?
/*
	bool avsSendTextMessage(const tstring & sFriendName, const tstring & sMsg);
	bool avsSendTextMessageP2P(const tstring & sFriendName, const tstring & sMsg);	// P2PType=im-text
	bool avsSendP2PData(const tstring & sFriendName, const tstring & sP2PType, const unsigned char * pData, unsigned int nSize);

	// Transfer manager
	bool avsSendFileTransfer(const tstring & sFriendName, const tstring & sFileName);		// ?
	bool avsResponseTransfer(const tstring & sFriendName, const tstring & sTransferType);	// ?
	bool avsTransferFile(const tstring & sFriendName, const tstring & sFileName);			// ?

	bool avsDownFile(const tstring & sFileName);
	bool avsUpFile(const tstring & sFileName);
*/
protected:
	bool avsResponseP2PUser(const tstring & sResponseUser, long nP2PType, long nP2PParam = 0);
	bool sendP2PAck(CDoP2PClientHandler::pointer pP2PClient, long nP2PType, long nP2PParam, bool bAckAck);

public:
	//CStreamCgcProxy(CStreamHandler * handler);
	CStreamCgcProxy(void);
	~CStreamCgcProxy(void);

private:
	// CgcClientHandler
	virtual void OnCgcResponse(const cgcParser & response);

	// OnRtpHandler
	//virtual void onReceiveEvent(const BYTE* pFramedata, int nFrameLen, u_long destIp, u_int destPort);
};

#endif // __StreamCgcProxy_h__
