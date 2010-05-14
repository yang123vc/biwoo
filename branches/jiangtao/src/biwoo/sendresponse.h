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

// sendresponse.h file here
#ifndef __sendresponse_h__
#define __sendresponse_h__

#include <CGCBase/includeapp.h>
using namespace cgc;
#include "../biwooinfo/offlineevent.h"
#include "../biwooinfo/accountinfo.h"
#include "../biwooinfo/datagroup.h"
#include "../biwooinfo/companyinfo.h"
#include "../biwooinfo/datainfo.h"
#include "../biwooinfo/frominfo.h"

bool sendDiaQuit(cgcResponse::pointer response, CUserInfo::pointer fromUser, CDialogInfo::pointer dialogInfo);
bool sendDiaQuit(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, CDialogInfo::pointer dialogInfo);
bool sendDiaInvite(cgcResponse::pointer response, CUserInfo::pointer fromUser, CDialogInfo::pointer dialogInfo, CUserInfo::pointer inviteUserInfo);
bool sendDiaInvite(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, CDialogInfo::pointer dialogInfo, CUserInfo::pointer inviteUserInfo);
bool sendDiaMembers(cgcResponse::pointer response, CUserInfo::pointer fromUser, CDialogInfo::pointer dialogInfo);
bool sendDiaMembers(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, CDialogInfo::pointer dialogInfo);

bool sendFileRequest(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, long mid, const std::string & filename, long filesize);
bool sendMsgRequest(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, long mid, long type);

// sendOfflineEvent
bool sendOfflineEvent(cgcResponse::pointer response, COfflineEvent::pointer offlineEvent);

// sendUserOfflineEvent
bool sendUserOnOfflineEvent(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, bool online, bool quit = false);

// sendDataGroupInfo
bool sendDataGroupInfo(cgcResponse::pointer response, CDataGroup::pointer dataGroup);

// sendDataInfo
bool sendDataInfo(cgcResponse::pointer response, CDataInfo::pointer dataInfo);

// sendCompanyInfo
bool sendCompanyInfo(cgcResponse::pointer response, CCompanyInfo::pointer companyInfo);

// sendCoGroupInfo
bool sendCoGroupInfo(cgcResponse::pointer response, unsigned int coId, CCoGroupInfo::pointer cogroupInfo, bool isMyCoGroup);

// sendCoGroupUserInfo
bool sendCoGroupUserInfo(cgcResponse::pointer response, unsigned int coId, unsigned cogroupId, CUserInfoPointer userInfo);

// sendMsg
// ????
//bool sendMsg(cgcResponse::pointer response, CUserInfo::pointer fromUser, long mid, long type, const char * msg, size_t size, bool bNew);
bool sendMsg(cgcResponse::pointer response, CUserInfo::pointer fromUser, CFromInfo::pointer dialogInfo, long mid, long type, cgcAttachment::pointer attachment, bool bNew, time_t offeventTime = 0);
bool sendMsg(cgcResponse::pointer response, CUserInfo::pointer fromUser, CFromInfo::pointer dialogInfo, long mid, cgcAttachment::pointer attachment);
bool sendMsgImage(cgcResponse::pointer response, CUserInfo::pointer fromUser, CFromInfo::pointer dialogInfo, long mid, long width, long height, long type, cgcAttachment::pointer attachment, bool bNew, time_t offeventTime = 0);
//bool sendMsg(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, long mid, long type, const char * msg, size_t size, bool bNew);
bool sendMsg(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, CFromInfo::pointer dialogInfo, long mid, cgcAttachment::pointer attachment);
bool sendMsg(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, CFromInfo::pointer dialogInfo, long mid, long type, cgcAttachment::pointer attachment, bool bNew);
bool sendMsgImage(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, CFromInfo::pointer dialogInfo, long mid, long width, long height, long type, cgcAttachment::pointer attachment, bool bNew);

bool getUserInfo(CAccountInfo::pointer accountInfo, const std::string & userAccount, CUserInfo::pointer & resultUserInfo);

#endif // __sendresponse_h__

