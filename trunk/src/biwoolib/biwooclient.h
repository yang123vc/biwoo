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

// biwooclient.h file here
#ifndef __biwooclient_h__
#define __biwooclient_h__

#include "dlldefine.h"
#include "biwoolibhandler.h"
#include "../biwooinfo/accconversation.h"
#include "../biwooinfo/frominfo.h"

class BIWOOLIB_CLASS CBiwooClient
{
public:
	void setHandler(CbiwooHandler * handler);
	bool start(const CCgcAddress & serverAddr, const CCgcAddress & fileAddr, const CCgcAddress & p2prtpAddr, const CCgcAddress & p2pudpAddr, CbiwooHandler * handler);
	void stop(void);
	bool isStarted(void) const;
	bool isOpenSession(void) const;

	const CLockList<CFromInfo::pointer>	& getRecents(void) const;
	CAccountConversation::pointer getAccountConversations(CFromInfo::pointer fromInfo);
	CConversationInfo::pointer getUnreadConversation(void);
	void deleteUserConversation(CFromInfo::pointer fromInfo);

	CUserInfo::pointer getUserInfo(const std::string & userAccount) const;
	CUserInfo::pointer getUserInfo(const std::string & userAccount, CCoGroupInfo::pointer & outCogroupInfo) const;
	CCoGroupInfo::pointer getUserInfo(CUserInfo::pointer userAccount) const;

	///////////////////////////////////////////////////
	// Account Manager
	//bool createAccount(CUserInfo::pointer newAccount);
	//bool destroyAccount(const tstring & accountName);
	bool isRegistered(void) const;
	CAccountInfo::pointer currentAccount(void) const;
	bool accountRegister(const tstring & accountName, const tstring & password);
	//bool accountRegConfirm(void);
	bool accountUnRegister(void);
	bool accountLoad(void);
	bool changePassword(const tstring & oldPassword, const tstring & newPassword);
	bool changeNick(const tstring & newNick);

	///////////////////////////////////////////////////
	// Friend Manager
	//bool createFriendGroup(CGroupInfo::pointer groupInfo);
	//bool deleteFriendGroup(unsigned int groupId);
	//bool groupChangeName(long groupId, const tstring & newName);
	//bool friendLoad(void);

	// video call
	bool openLocalAV(HWND hWndPreview);
	void closeLocalAV(void);
	void moveRemoteWindow(void);

	bool videoCall(CUserInfo::pointer calltoUser, HWND hWndPreview);
	bool acceptVideoCall(CUserInfo::pointer responsetoUser, HWND hWndPreview, long mid);
	bool releaseVideoCall(CUserInfo::pointer responsetoUser, long mid);

	///////////////////////////////////////////////////
	// Company Manager
	//bool coLoad(void);
	//bool coLoadGroup(void);

	///////////////////////////////////////////////////
	// P2P Manager
	//int p2pRequest(const tstring & sRequestUser, const tstring & sP2PType, const tstring & sP2PParam = _T(""));
	//int p2pDisconnect(const tstring & sP2PUser, const tstring & sP2PType);

	// Max 6000 size.
	bool msgSendText(CFromInfo::pointer sendtoUser, const char * sMsg, size_t size, bool bNew);
	bool msgSendImage(CFromInfo::pointer sendtoUser, int width, int height, int imagetype, const char * sMsg, size_t size, bool bNew);
	bool msgSendFile(CUserInfo::pointer sendtoUser, const char * filepath, const char * filename);
	bool msgAcceptFile(CUserInfo::pointer responsetoUser, long mid, const char * savetofilepath, const char * filename);
	bool msgRejectFile(CUserInfo::pointer responsetoUser, long mid);
	bool msgCancelFile(CUserInfo::pointer responsetoUser, long mid);


	///////////////////////////////////////////////////
	// Dialog Manager
	bool diaInvite(long dialogId, const std::list<CUserInfo::pointer> & users);
	bool diaQuit(long dialogId);

/*
	// P2P

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

	// message
	//bool avsSendTextMessageP2P(const tstring & sFriendName, const tstring & sMsg);	// P2PType=im-text
	*/

public:
	CBiwooClient(void);
	~CBiwooClient(void);
};

#endif // __biwooclient_h__
