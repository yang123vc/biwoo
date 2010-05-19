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


#define BIWOOLIB_EXPORTS

#include "biwooclient.h"

#include "BiwooCgcProxy.h"

////////////////////////////////////////////////
//
CBiwooCgcProxy gBiwooProxy;

CBiwooClient::CBiwooClient(void)

{
}

CBiwooClient::~CBiwooClient(void)
{
}

void CBiwooClient::setHandler(CbiwooHandler * handler)
{
	gBiwooProxy.setHandler(handler);
}

bool CBiwooClient::start(const CCgcAddress & sServerAddr, const CCgcAddress & fileAddr, const CCgcAddress & p2pAddr, const CCgcAddress & p2pudpAddr, CbiwooHandler * pHandler)
{
	gBiwooProxy.setHandler(pHandler);
	return gBiwooProxy.start(sServerAddr, fileAddr, p2pAddr, p2pudpAddr);
}

void CBiwooClient::stop(void)
{
	gBiwooProxy.setHandler(NULL);
	gBiwooProxy.stop();
}

bool CBiwooClient::isStarted(void) const
{
	return gBiwooProxy.isStarted();
}

bool CBiwooClient::isOpenSession(void) const
{
	return gBiwooProxy.isOpenSession();
}

const CLockList<CFromInfo::pointer>	& CBiwooClient::getRecents(void) const
{
	return gBiwooProxy.getRecents();
}
CAccountConversation::pointer CBiwooClient::getAccountConversations(CFromInfo::pointer fromInfo)
{
	return gBiwooProxy.getAccountConversations(fromInfo);
}
CConversationInfo::pointer CBiwooClient::getUnreadConversation(void)
{
	return gBiwooProxy.getUnreadConversation();
}
void CBiwooClient::deleteUserConversation(CFromInfo::pointer fromInfo)
{
	gBiwooProxy.deleteUserConversation(fromInfo);
}

CUserInfo::pointer CBiwooClient::getUserInfo(const std::string & userAccount) const
{
	return gBiwooProxy.getUserInfo(userAccount);
}

CUserInfo::pointer CBiwooClient::getUserInfo(const std::string & userAccount, CCoGroupInfo::pointer & outCogroupInfo) const
{
	return gBiwooProxy.getUserInfo(userAccount, outCogroupInfo);
}

CCoGroupInfo::pointer CBiwooClient::getUserInfo(CUserInfo::pointer userAccount) const
{
	return gBiwooProxy.getUserInfo(userAccount);
}

bool CBiwooClient::isRegistered(void) const
{
	return gBiwooProxy.isRegistered();
}

CAccountInfo::pointer CBiwooClient::currentAccount(void) const
{
	return gBiwooProxy.getCurrentAccount();
}

//bool CBiwooClient::createAccount(CUserInfo::pointer newAccount)
//{
//	return gBiwooProxy.createAccount(newAccount);
//}

bool CBiwooClient::accountRegister(const tstring & sUsername, const tstring & sPassword)
{
	return gBiwooProxy.accountRegister(sUsername, sPassword);
}

//bool CBiwooClient::accountRegConfirm(void)
//{
//	return gBiwooProxy.accountRegConfirm();
//}

bool CBiwooClient::accountUnRegister(void)
{
	return gBiwooProxy.accountUnRegister();
}

bool CBiwooClient::accountLoad(void)
{
	return gBiwooProxy.accountLoad();
}
//
//bool CBiwooClient::createFriendGroup(CGroupInfo::pointer groupInfo)
//{
//	return gBiwooProxy.createFriendGroup(groupInfo);
//}
//
//bool CBiwooClient::deleteFriendGroup(unsigned int groupId)
//{
//	return gBiwooProxy.deleteFriendGroup(groupId);
//}
//
//bool CBiwooClient::groupChangeName(long groupId, const tstring & newName)
//{
//	return gBiwooProxy.groupChangeName(groupId, newName);
//}

bool CBiwooClient::openLocalAV(HWND hWndPreview)
{
	return gBiwooProxy.openLocalAV(hWndPreview);
}

void CBiwooClient::closeLocalAV(void)
{
	gBiwooProxy.closeLocalAV();
}

void CBiwooClient::moveRemoteWindow(void)
{
	gBiwooProxy.moveRemoteWindow();
}

bool CBiwooClient::videoCall(CUserInfo::pointer calltoUser, HWND hWndPreview)
{
	return gBiwooProxy.videoCall(calltoUser, hWndPreview);
}

bool CBiwooClient::acceptVideoCall(CUserInfo::pointer responsetoUser, HWND hWndPreview, long mid)
{
	return gBiwooProxy.acceptVideoCall(responsetoUser, hWndPreview, mid);
}
bool CBiwooClient::releaseVideoCall(CUserInfo::pointer responsetoUser, long mid)
{
	return gBiwooProxy.releaseVideoCall(responsetoUser, mid);
}

bool CBiwooClient::msgSendText(CFromInfo::pointer sendtoUser, const char * sMsg, size_t size, bool bNew)
{
	return gBiwooProxy.msgSendText(sendtoUser, sMsg, size, bNew);
}
bool CBiwooClient::msgSendImage(CFromInfo::pointer sendtoUser, int width, int height, int imagetype, const char * sMsg, size_t size, bool bNew)
{
	return gBiwooProxy.msgSendImage(sendtoUser, width, height, imagetype, sMsg, size, bNew);
}
bool CBiwooClient::msgSendFile(CUserInfo::pointer sendtoUser, const char * filepath, const char * filename)
{
	return gBiwooProxy.msgSendFile(sendtoUser, filepath, filename);
}

bool CBiwooClient::msgAcceptFile(CUserInfo::pointer responsetoUser, long mid, const char * savetofilepath, const char * filename)
{
	return gBiwooProxy.msgAcceptFile(responsetoUser, mid, savetofilepath, filename);
}
bool CBiwooClient::msgRejectFile(CUserInfo::pointer responsetoUser, long mid)
{
	return gBiwooProxy.msgRejectFile(responsetoUser, mid);
}

bool CBiwooClient::msgCancelFile(CUserInfo::pointer responsetoUser, long mid)
{
	return gBiwooProxy.msgCancelFile(responsetoUser, mid);
}

bool CBiwooClient::diaInvite(long dialogId, const std::list<CUserInfo::pointer> & users)
{
	return gBiwooProxy.diaInvite(dialogId, users);
}

bool CBiwooClient::diaQuit(long dialogId)
{
	return gBiwooProxy.diaQuit(dialogId);
}

/*
bool CBiwooClient::createConference(const tstring & conferenceName)
{
	return gBiwooProxy.createConference(conferenceName);
}

bool CBiwooClient::enableAudioSend(const tstring & conferenceName, int memberIndex, bool enable)
{
	return gBiwooProxy.enableAudioSend(conferenceName, memberIndex, enable);
}

bool CBiwooClient::enableAudioRecv(const tstring & conferenceName, int memberIndex, bool enable)
{
	return gBiwooProxy.enableAudioRecv(conferenceName, memberIndex, enable);
}

bool CBiwooClient::enableVideoSend(const tstring & conferenceName, int memberIndex, bool enable)
{
	return gBiwooProxy.enableVideoSend(conferenceName, memberIndex, enable);
}

bool CBiwooClient::enableVideoRecv(const tstring & conferenceName, int memberIndex, bool enable)
{
	return gBiwooProxy.enableVideoRecv(conferenceName, memberIndex, enable);
}
*/
