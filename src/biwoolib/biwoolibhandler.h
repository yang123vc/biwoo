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

// biwoolibhandler.h file here
#ifndef __biwoolibhandler_h__
#define __biwoolibhandler_h__

#include "stl/stldef.h"
#include "CGCLib/CGCLib.h"
#include "../biwooinfo/dialoginfo.h"
#include "../biwooinfo/conversationinfo.h"
#include "../biwooinfo/accountinfo.h"
#include "../biwooinfo/companyinfo.h"

class DoP2PClientHandler
{
public:
	virtual const tstring & doGetP2PUser(void) const = 0;
	virtual const tstring & doGetP2PType(void) const = 0;
	virtual const tstring & doGetP2PParam(void) const = 0;
	virtual void doSetMediaType(unsigned short mediaType) = 0;
	virtual size_t doSendP2PData(const unsigned char * data, size_t size, unsigned long timestamp) = 0;

};

class P2PHandler
{
public:
	virtual void onP2PEvent(DoP2PClientHandler * p2pClient, CCgcData::pointer receiveData) = 0;
};

class CbiwooHandler
{
public:
	enum LogoutType
	{
		LT_Normal				= 0x1
		, LT_LoginAnotherPlace
		, ServerQuit
	};

	virtual void onRegConfirm(void) {}
	virtual void onUserInfo(const tstring & sUserId, const tstring & sUserName) {}
	virtual void onUserLogined(CAccountInfo::pointer accountInfo) {}	// Owner
	virtual void onUserLogined(CFriendInfo::pointer friendInfo) {}		// Friend
	virtual void onUserLogined(CCoGroupInfo::pointer cogroupInfo, CUserInfo::pointer userInfo) {}
	virtual void onUserLoginError(long errorCode) {}
	virtual void onUserLogouted(CAccountInfo::pointer accountInfo, LogoutType logoutType) {}	// Owner
	virtual void onUserLogouted(CFriendInfo::pointer friendInfo) {}		// Friend
	virtual void onUserLogouted(CCoGroupInfo::pointer cogroupInfo, CUserInfo::pointer userInfo) {}

	virtual void onGroupInfo(bool createReturn, CGroupInfo::pointer groupInfo) {}
	virtual void onFriendLoadReturned(long resultValue) {}
	virtual void onFriendGroupDelete(long resultValue) {}
	virtual void onFriendGroupDelete(CGroupInfo::pointer groupInfo) {}

	virtual void onCoLoadGroupReturned(long resultValue) {}
	virtual void onCompanyInfo(CCompanyInfo::pointer companyInfo) {}
	virtual void onCoGroupInfo(CCoGroupInfo::pointer cogroupInfo, bool isMyCoGroup) {}
	virtual void onCoGroupUser(CCoGroupInfo::pointer cogroupInfo, CUserInfoPointer userInfo, bool isMyCoGroupUser) {}

	//virtual void onSendMessage(CCoGroupInfo::pointer cogroupInfo, CConversationInfo::pointer conversationInfo) {}

	virtual void onP2PRequestResult(const tstring & sFromUser, const tstring & sP2PType, long resuleValue) {}
	virtual P2PHandler * onP2PUserAck(DoP2PClientHandler * p2pClient) {return 0;}
	virtual void onP2PRequestTimeout(DoP2PClientHandler * p2pClient) {}
	virtual void onP2PUserDisconnect(DoP2PClientHandler * p2pClient) {}

	virtual void onDialogInvited(CDialogInfo::pointer dialogInfo, CUserInfo::pointer inviteUserInfo) {}

	//virtual void onSendFileRequest(CUserInfo::pointer fromUserInfo, long mid, const tstring & sFilename, long nFilesize) {}

};


#endif // __biwoolibhandler_h__
