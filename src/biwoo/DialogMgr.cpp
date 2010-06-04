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

// DialogMgr.cpp : Defines the initialization routines for the DLL.
//

// cgcÍ·ÎÄ¼þ
#include <CGCBase/includeapp.h>
using namespace cgc;

///////////////////////////////
// 
#include "AVSProxy.h"
#include "sendresponse.h"

extern "C" int CGC_API DiaInvite(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	/////////////////////////////////
	// Request
	const tstring & sAccountId = request->getParameterValue(_T("AccountId"), _T(""));
	if (sAccountId.empty()) return -1;
	long dialogId = request->getParameterValue(_T("DID"), 0);

	/////////////////////////////////
	// Process
	CAccountInfo::pointer accountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTIDS, sAccountId));
	if (accountInfo.get() == NULL)
	//if (!gAVSProxy->m_accountids.find(sAccountId, accountInfo))
	{
		// Un register.
		return 14;
	}else if (session->getId().compare(accountInfo->getSessionId()) != 0)
	{
		// SessionId Error
		return -3;
	}

	CDialogInfo::pointer dialogInfo;
	if (dialogId == 0)
	{
		// New Dialog
		dialogInfo = CDialogInfo::create(gAVSProxy->getNextDialogId(), accountInfo->getUserinfo());
		dialogInfo->m_members.insert(accountInfo->getUserinfo()->getAccount(), accountInfo->getUserinfo());
		gApplication->setAttribute(BMT_DIALOGS, dialogInfo->dialogId(), dialogInfo);
		//gAVSProxy->m_dialogs.insert(dialogInfo->dialogId(), dialogInfo);

		response->setParameter(cgcParameter::create(_T("DID"), dialogInfo->dialogId()));
		response->sendResponse();
	}else
	{
		dialogInfo = CGC_POINTER_CAST<CDialogInfo>(gApplication->getAttribute(BMT_DIALOGS, dialogId));
		if (dialogInfo.get() == NULL)
		//if (!gAVSProxy->m_dialogs.find(dialogId, dialogInfo))
		{
			// Dialog id not exist.
			return 71;
		}/*else if (dialogInfo->manager().get() != accountInfo->getUserinfo().get())
		{
			// ?? Not the manager error.
			return 72;
		}*/
	}

	int index = 0;
	while (true)
	{
		char buffer[20];
		sprintf(buffer, "UA%d", index++);

		const tstring & sInviteUserAccount = request->getParameterValue(buffer, _T(""));
		if (sInviteUserAccount.empty())
		{
			break;
		}

		if (dialogInfo->m_members.exist(sInviteUserAccount))
		{
			// already exist in the dialog.
			continue;
		}

		CUserInfo::pointer inviteUserInfo;
		if (!getUserInfo(accountInfo, sInviteUserAccount, inviteUserInfo))
		{
			// ? continue;
			break;
		}

		// 1 Send the dialog members that invite account event.
		CLockMap<std::string, CUserInfo::pointer>::const_iterator iterUserInfo;
		boost::mutex::scoped_lock lockUserInfo(const_cast<boost::mutex&>(dialogInfo->m_members.mutex()));
		for (iterUserInfo=dialogInfo->m_members.begin(); iterUserInfo!=dialogInfo->m_members.end(); iterUserInfo++)
		{
			CUserInfo::pointer memberUserInfo = iterUserInfo->second;

			// Not register.
			CAccountInfo::pointer memberAccountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTS, memberUserInfo->getAccount()));
			if (memberAccountInfo.get() == NULL)
			//if (!gAVSProxy->m_accounts.find(memberUserInfo->getAccount(), memberAccountInfo))
			{
				// offline event
				CMessageInfo::pointer messageInfo = CMessageInfo::create(0, 0, CConversationInfo::CT_INVITE, true);
				COfflineEvent::pointer offlineEvent = COfflineEvent::create(601, CFromInfo::create(dialogInfo), accountInfo->getUserinfo(), messageInfo);
				offlineEvent->toAccount(inviteUserInfo);
				gAVSProxy->addOffEvent(memberUserInfo, offlineEvent);
				continue;
			}

			sendDiaInvite(memberAccountInfo, accountInfo->getUserinfo(), dialogInfo, inviteUserInfo);
		}
		lockUserInfo.unlock();

		// 2 Send invite useraccount the dialog members info.
		CAccountInfo::pointer inviteAccountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTS, sInviteUserAccount));
		if (inviteAccountInfo.get() != NULL)
		{
			// 
			sendDiaMembers(inviteAccountInfo, accountInfo->getUserinfo(), dialogInfo);
		}else
		{
			// Not register.
			// offline event
			CLockMap<std::string, CUserInfo::pointer>::const_iterator iterUserInfo;
			boost::mutex::scoped_lock lockUserInfo(const_cast<boost::mutex&>(dialogInfo->m_members.mutex()));
			for (iterUserInfo=dialogInfo->m_members.begin(); iterUserInfo!=dialogInfo->m_members.end(); iterUserInfo++)
			{
				CUserInfo::pointer memberUserInfo = iterUserInfo->second;

				CMessageInfo::pointer messageInfo = CMessageInfo::create(0, 0, CConversationInfo::CT_INVITE, true);
				COfflineEvent::pointer offlineEvent = COfflineEvent::create(601, CFromInfo::create(dialogInfo), accountInfo->getUserinfo(), messageInfo);
				offlineEvent->toAccount(memberUserInfo);
				gAVSProxy->addOffEvent(inviteUserInfo, offlineEvent);
			}
		}

		dialogInfo->m_members.insert(inviteUserInfo->getAccount(), inviteUserInfo);		
	}

	// Response
	return 0;
}

extern "C" int CGC_API DiaMember(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	/////////////////////////////////
	// Request
	const tstring & sAccountId = request->getParameterValue(_T("AccountId"), _T(""));
	if (sAccountId.empty()) return -1;
	long dialogId = request->getParameterValue(_T("DID"), 0);
	if (dialogId == 0) return -1;

	/////////////////////////////////
	// Process
	CAccountInfo::pointer accountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTIDS, sAccountId));
	if (accountInfo.get() == NULL)
	//if (!gAVSProxy->m_accountids.find(sAccountId, accountInfo))
	{
		// Un register.
		return 14;
	}else if (session->getId().compare(accountInfo->getSessionId()) != 0)
	{
		// SessionId Error
		return -3;
	}

	CDialogInfo::pointer dialogInfo = CGC_POINTER_CAST<CDialogInfo>(gApplication->getAttribute(BMT_DIALOGS, dialogId));
	int index = 0;
	while (true)
	{
		char buffer[20];
		sprintf(buffer, "UA%d", index++);

		const tstring & memberUserAccount = request->getParameterValue(buffer, _T(""));
		if (memberUserAccount.empty())
		{
			break;
		}

		CUserInfo::pointer memberUserInfo;
		if (!getUserInfo(accountInfo, memberUserAccount, memberUserInfo))
		{
			// ? continue;
			break;
		}

		if (dialogInfo.get() == NULL)
		{
			dialogInfo = CDialogInfo::create(dialogId, memberUserInfo);
			dialogInfo->history(true);
			gApplication->setAttribute(BMT_DIALOGS, dialogInfo->dialogId(), dialogInfo);
		}

		if (dialogInfo->m_members.exist(memberUserAccount))
		{
			// already exist in the dialog.
			continue;
		}
		dialogInfo->m_members.insert(memberUserAccount, memberUserInfo);
	}

	// Response
	return 0;
}

extern "C" int CGC_API DiaQuit(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	/////////////////////////////////
	// Request
	const tstring & sAccountId = request->getParameterValue(_T("AccountId"), _T(""));
	if (sAccountId.empty()) return -1;
	long dialogId = request->getParameterValue(_T("DID"), 0);

	/////////////////////////////////
	// Process
	CAccountInfo::pointer accountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTIDS, sAccountId));
	if (accountInfo.get() == NULL)
	{
		// Un register.
		return 14;
	}else if (session->getId().compare(accountInfo->getSessionId()) != 0)
	{
		// SessionId Error
		return -3;
	}

	CDialogInfo::pointer dialogInfo = CGC_POINTER_CAST<CDialogInfo>(gApplication->getAttribute(BMT_DIALOGS, dialogId));
	if (dialogInfo.get() == NULL)
	{
		// Dialog id not exist.
		return 71;
	}

	if (!dialogInfo->m_members.remove(accountInfo->getUserinfo()->getAccount()))
	{
		// Member not exist.
		return 73;
	}

	CLockMap<std::string, CUserInfo::pointer>::const_iterator iterUserInfo;
	boost::mutex::scoped_lock lockUserInfo(const_cast<boost::mutex&>(dialogInfo->m_members.mutex()));
	for (iterUserInfo=dialogInfo->m_members.begin(); iterUserInfo!=dialogInfo->m_members.end(); iterUserInfo++)
	{
		CUserInfo::pointer memberUserInfo = iterUserInfo->second;

		// Not register.
		CAccountInfo::pointer memberAccountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTS, memberUserInfo->getAccount()));
		if (memberAccountInfo.get() == NULL)
		{
			// offline event
			CMessageInfo::pointer messageInfo = CMessageInfo::create(0, 0, CConversationInfo::CT_QUIT, true);
			COfflineEvent::pointer offlineEvent = COfflineEvent::create(602, CFromInfo::create(dialogInfo), accountInfo->getUserinfo(), messageInfo);
			offlineEvent->toAccount(memberUserInfo);
			gAVSProxy->addOffEvent(memberUserInfo, offlineEvent);
			continue;
		}

		sendDiaQuit(memberAccountInfo, accountInfo->getUserinfo(), dialogInfo);
	}
	lockUserInfo.unlock();

	// Response
	return 0;
}
