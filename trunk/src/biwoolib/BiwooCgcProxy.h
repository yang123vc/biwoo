/*
    Biwoo is an enterprise communication software base on CGCP.
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

// biwooCgcproxy.h file here
#pragma once
#ifndef __biwooCgcproxy_h__
#define __biwooCgcproxy_h__

#include <CGCLib/CGCLib.h>
#include <CGCBase/cgcString.h>
#include "../libStreamX/libStreamX.h"

#include "biwoolibhandler.h"
#include "../biwooinfo/accountinfo.h"
#include "../biwooinfo/companyinfo.h"
#include "../biwooinfo/accconversation.h"
#include "../biwooinfo/usermessages.h"
#include "../biwooinfo/sendinfo.h"
#include "../biwooinfo/frominfo.h"

////////////////////////////
// const 
const unsigned short const_CGCMediaType	= 1;

const tstring const_Avs_AppName					= _T("biwoo");

const tstring const_Api_LoadSetting				= _T("0101");
const ULONG const_CallSign_LoadSetting			= 0x0101;

// account manager
const tstring const_Api_AccCreate				= _T("1001");
const ULONG const_CallSign_AccCreate			= 0x1001;
const tstring const_Api_AccDestroy				= _T("1002");
const ULONG const_CallSign_AccDestroy			= 0x1002;
const tstring const_Api_Register				= _T("1011");
const ULONG const_CallSign_Register				= 0x1011;
const tstring const_Api_RegConfirm				= _T("1012");
const ULONG const_CallSign_RegConfirm			= 0x1012;
const tstring const_Api_UnRegister				= _T("1013");
const ULONG const_CallSign_UnRegister			= 0x1013;
const tstring const_Api_AccLoad					= _T("1014");
const ULONG const_CallSign_AccLoad				= 0x1014;

// Friend Manager
const tstring const_Api_FriRequest				= _T("2001");
const ULONG const_CallSign_FriRequest			= 0x2001;
const tstring const_Api_FriResponse				= _T("2002");
const ULONG const_CallSign_FriResponse			= 0x2002;
const tstring const_Api_FriDelete				= _T("2003");
const ULONG const_CallSign_FriDelete			= 0x2003;
const tstring const_Api_FriNote					= _T("2004");
const ULONG const_CallSign_FriNote				= 0x2004;
const tstring const_Api_FriSetTo				= _T("2005");
const ULONG const_CallSign_FriSetTo				= 0x2005;
const tstring const_Api_FriLoad					= _T("2006");
const ULONG const_CallSign_FriLoad				= 0x2006;
const tstring const_Api_FriGrpCreate			= _T("2011");
const ULONG const_CallSign_FriGrpCreate			= 0x2011;
const tstring const_Api_FriGrpDestroy			= _T("2012");
const ULONG const_CallSign_FriGrpDestroy		= 0x2012;
const tstring const_Api_FriGrpSetName			= _T("2013");
const ULONG const_CallSign_FriGrpSetName		= 0x2013;
const tstring const_Api_FriGrpLoad				= _T("2014");
const ULONG const_CallSign_FriGrpLoad			= 0x2014;


// Company manager
const tstring const_Api_CoLoad					= _T("3001");
const ULONG const_CallSign_CoLoad				= 0x3001;
const tstring const_Api_CoLoadGroup				= _T("3002");
const ULONG const_CallSign_CoLoadGroup			= 0x3002;
const tstring const_Api_CoLoadGroupUser			= _T("3003");
const ULONG const_CallSign_CoLoadGroupUser		= 0x3003;


// Message manager
const tstring const_Api_MsgSend					= _T("6101");
const ULONG const_CallSign_MsgSend				= 0x6101;
const tstring const_Api_MsgRequest				= _T("6111");
const ULONG const_CallSign_MsgRequest			= 0x6111;
const tstring const_Api_MsgResponse				= _T("6112");
const ULONG const_CallSign_MsgResponse			= 0x6112;
const tstring const_Api_MsgSenddata				= _T("6113");
const ULONG const_CallSign_MsgSenddata			= 0x6113;


// Dialog manager
const tstring const_Api_DiaInvite				= _T("7001");
const ULONG const_CallSign_DiaInvite			= 0x7001;
const tstring const_Api_DiaMember				= _T("7002");
const ULONG const_CallSign_DiaMember			= 0x7002;
const tstring const_Api_DiaQuit					= _T("7003");
const ULONG const_CallSign_DiaQuit				= 0x7003;


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

// ???
const tstring const_Avs_Api_GetAllUser			= _T("GetAllUser");		// ?
const ULONG const_CallSign_GetAllUser			= 0x0111;


///////////////////////////////
// handler

class CBiwooCgcProxy
	: public CgcClientHandler
	, public OnP2PHandler
	//, public OnRtpHandler
{
private:
	CbiwooHandler * m_handler;
	CSotpClient m_sotpClient;
	DoSotpClientHandler::pointer m_cgcClient;
	DoSotpClientHandler::pointer m_fileClient;

	bool m_bDoAccountUnRegister;
	bool m_bLoadSettingReturned;
	CAccountInfo::pointer m_account;
	CCgcAddress m_serverAddr;
	CCgcAddress m_fileServerAddr;
	std::string m_sModulePath;

	CP2PProxy		m_p2pav;
	DoDSHandler::pointer	m_LDoDSHandler;
	DoDSHandler::pointer	m_RDoDSHandler;

	CLockMap<long, CDialogInfo::pointer> m_dialogs;
	CLockMap<tstring, CAccountConversation::pointer> m_avs;	// account conversations
	CLockMap<long, CAccountConversation::pointer> m_avs2;	// account conversations
	CLockMap<long, CMessageInfo::pointer> m_messageinfos;
	CLockMap<long, bool> m_rejects;
	CLockMap<long, CSendInfo::pointer> m_sends;
	CLockMap<long, CSendInfo::pointer> m_recvs;
	
	CLockMapPtr<long, boost::thread*> m_sendthreads;	// MID ->
	CLockList<CConversationInfo::pointer>	m_unreads;
	CLockList<CFromInfo::pointer>	m_recents;

	long			m_currentMID;	// messageID

public:
	//const CgcBaseClient & getCgcClient(void) const {return m_cgcClient;}
	void setHandler(CbiwooHandler * newv) {m_handler = newv;}

	bool start(const CCgcAddress & serverAddr);
	void stop(void);
	bool isStarted(void) const {return m_cgcClient != NULL;}
	bool isOpenSession(void) const {return m_cgcClient == NULL ? false : m_cgcClient->doIsSessionOpened();}
	
	bool avsLoadSetting(void);
	
	const CLockList<CFromInfo::pointer>	& getRecents(void) const {return m_recents;}
	CAccountConversation::pointer getAccountConversations(CFromInfo::pointer userInfo);
	CDialogInfo::pointer getDialogInfo(long dialogid);
	CConversationInfo::pointer getUnreadConversation(void);
	bool hasUnread(void) const;
	void deleteUserConversation(CFromInfo::pointer fromInfo);

	CUserInfo::pointer getUserInfo(const std::string & userAccount) const;
	CUserInfo::pointer getUserInfo(const std::string & userAccount, CCoGroupInfo::pointer & outCogroupInfo) const;
	CCoGroupInfo::pointer getUserInfo(CUserInfo::pointer userAccount) const;


	//////////////////////////////////////////////////
	// Friend Manager
	bool createFriendGroup(CGroupInfo::pointer groupInfo);
	bool deleteFriendGroup(unsigned int groupId);
	bool groupChangeName(long groupId, const tstring & newName);
	bool friendLoad(void);

	//////////////////////////////////////////////////
	// Company Manager
	bool coLoad(void);
	bool coLoadGroup(unsigned int coId);
	bool coLoadGroupUser(unsigned int coId, unsigned int cogroupId);

	//////////////////////////////////////////////////
	// Account Manager
	//bool createAccount(const tstring & accountName, const tstring & sPassword);
	bool isRegistered(void) const {return m_account.get() != 0;}
	CAccountInfo::pointer getCurrentAccount(void) const {return m_account;}
	bool accountRegister(const tstring & accountName, const tstring & sPassword);
	bool accountRegConfirm(void);
	bool accountUnRegister(void);
	bool accountLoad(void);

	// Conference
	bool createConference(const tstring & conferenceName);
	bool enableAudioSend(const tstring & conferenceName, int memberIndex, bool enable);
	bool enableAudioRecv(const tstring & conferenceName, int memberIndex, bool enable);
	bool enableVideoSend(const tstring & conferenceName, int memberIndex, bool enable);
	bool enableVideoRecv(const tstring & conferenceName, int memberIndex, bool enable);

	// video call
	bool openLocalAV(HWND hWndPreview);
	void closeLocalAV(void);
	void closeRemoteAV(void);
	void moveRemoteWindow(void);
	void reversalRemoteVideo(void);

	bool videoCall(CUserInfo::pointer calltoUser, HWND hWndPreview);
	bool acceptVideoCall(CUserInfo::pointer responsetoUser, HWND hWndPreview, long mid);
	bool rejectVideoCall(CUserInfo::pointer responsetoUser, long mid);
	bool releaseVideoCall(CUserInfo::pointer responsetoUser, long mid);

	bool msgSendText(CFromInfo::pointer sendtoUser, const char * sMsg, size_t size, bool bNew);
	bool msgSendImage(CFromInfo::pointer sendtoUser, int width, int height, int imagetype, const char * sMsg, size_t size, bool bNew);
	bool msgSendFile(CUserInfo::pointer sendtoUser, const char * filepath, const char * filename);
	bool msgAcceptFile(CUserInfo::pointer responsetoUser, long mid, const char * savetofilepath, const char * filename);
	bool msgRejectFile(CUserInfo::pointer responsetoUser, long mid);
	bool msgCancelFile(CUserInfo::pointer responsetoUser, long mid);

	void doProcSend(CSendInfo::pointer sendInfo);

	// Dialog manager
	bool diaInvite(long dialogId, const std::list<CUserInfo::pointer> & users);
	bool diaMember(long dialogId, const std::list<CUserInfo::pointer> & users);
	bool diaQuit(long dialogId);

protected:
	static void doStaProcSend(CBiwooCgcProxy * pProxy, CSendInfo::pointer sendInfo);

public:
	CBiwooCgcProxy(void);
	~CBiwooCgcProxy(void);

private:
	// OnP2PHandler
	virtual void onFileRequest(const tstring & sFromUser, long fid, const tstring & filename, size_t filesize);
	virtual void onFileRejected(const tstring & sFromUser, long fid);
	virtual void onFileCanceled(const tstring & sFromUser, long fid);
	virtual void onSendFileData(const tstring & sFromUser, long fid, float percent);
	virtual void onReceiveFileData(const tstring & sFromUser, long fid, float percent);

	// CgcClientHandler
	virtual void OnCgcResponse(const cgcParser & response);


	// OnRtpHandler
	//virtual void onReceiveEvent(const BYTE* pFramedata, int nFrameLen, u_long destIp, u_int destPort);
};

#endif // __biwooCgcproxy_h__
