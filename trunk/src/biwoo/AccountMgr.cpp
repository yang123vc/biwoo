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

// AccountMgr.cpp : Defines the initialization routines for the DLL.
//

#ifdef WIN32
#include <windows.h>
#endif

// cgcÍ·ÎÄ¼þ
#include <CGCBase/includeapp.h>
using namespace cgc;

///////////////////////////////
// 
#include "AVSProxy.h"
#include "sendresponse.h"

extern "C" int CGC_API AccCreate(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	BOOST_ASSERT (request.get() != 0);
	BOOST_ASSERT (response.get() != 0);
	BOOST_ASSERT (session.get() != 0);

	/////////////////////////////////
	// Request
	cgcParameter::pointer pAccount = request->getParameter(_T("Account"));
	if (pAccount.get() == 0 || pAccount->getValue().length() == 0) return -1;
	cgcParameter::pointer pPassword = request->getParameter(_T("Password"));
	if (pPassword.get() == 0) return -1;

	const tstring & sName = request->getParameterValue(_T("Name"), pAccount->getValue());
	const tstring & sNick = request->getParameterValue(_T("Nick"), sName);

	/////////////////////////////////
	// process
	if (gApplication->existAttribute(BMT_ALLUSERS, pAccount->getValue()))
	//if (gAVSProxy->m_users.exist(pAccount->getValue()))
	{
		// Already exist.
		return 10;
	}
	CUserInfo::pointer userInfo = CUserInfo::create(pAccount->getValue(), pPassword->getValue());
	userInfo->setUserName(sName);
	userInfo->setNick(sNick);
	//gAVSProxy->m_users.insert(pAccount->getValue(), userInfo);
	gAVSProxy->addUserinfo(userInfo);

	/////////////////////////////////
	// Response
	response->setParameter(cgcParameter::create(_T("Account"), pAccount->getValue()));
	response->setParameter(cgcParameter::create(_T("Name"), sName));
	return 0;
}

extern "C" int CGC_API AccDestroy(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	/////////////////////////////////
	// Request
	cgcParameter::pointer pAccount = request->getParameter(_T("Account"));
	if (pAccount.get() == 0 || pAccount->getValue().length() == 0) return -1;
	cgcParameter::pointer pPassword = request->getParameter(_T("Password"));
	if (pPassword.get() == 0) return -1;

	/////////////////////////////////
	// Process
	CUserInfo::pointer userInfo = CGC_POINTER_CAST<CUserInfo>(gApplication->getAttribute(BMT_ALLUSERS, pAccount->getValue()));
	if (userInfo.get() == NULL)
	//if (!gAVSProxy->m_users.find(pAccount->getValue(), userInfo, false))
	{
		// Not exist.
		return 11;
	}else if (userInfo->getPassword() != pPassword->getValue())
	{
		// Password Error.
		return 13;
	}
	//gAVSProxy->m_users.remove(pAccount->getValue());
	gAVSProxy->deleteUserinfo(pAccount->getValue());

	/////////////////////////////////
	// Response
	response->setParameter(cgcParameter::create(_T("Account"), pAccount->getValue()));
	return 0;
}

extern "C" int CGC_API AccRegister(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	/////////////////////////////////
	// Request
 	cgcParameter::pointer pAccount = request->getParameter(_T("Account"));
	if (pAccount.get() == 0 || pAccount->getValue().length() == 0) return -1;
	cgcParameter::pointer pPassword = request->getParameter(_T("Password"));
	if (pPassword.get() == 0) return -1;

	/////////////////////////////////
	// Process
	CUserInfo::pointer userInfo = CGC_POINTER_CAST<CUserInfo>(gApplication->getAttribute(BMT_ALLUSERS, pAccount->getValue()));
	if (userInfo.get() == NULL)
	//if (!gAVSProxy->m_users.find(pAccount->getValue(), userInfo, false))
	{
		// Not exist.
		return 11;
	}else if (userInfo->getPassword() != pPassword->getValue())
	{
		// Password Error.
		return 13;
	}

	tstring sAccountId(pAccount->getValue());
	sAccountId.append(_T("@"));
	sAccountId.append(gSystem->getServerAddr());

	CAccountInfo::pointer accountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTIDS, sAccountId));
	if (accountInfo.get() != NULL)
	//if (gAVSProxy->m_accountids.find(sAccountId, accountInfo))
	{
		// Set login another place event.
		sendUserOnOfflineEvent(accountInfo, accountInfo->getUserinfo(), false);
		accountInfo->setSessionId(session->getId());
	}else
	{
		accountInfo = CAccountInfo::create(userInfo, sAccountId);
		accountInfo->setSessionId(session->getId());
		gApplication->setAttribute(BMT_ACCOUNTIDS, sAccountId, accountInfo);
		gApplication->setAttribute(BMT_ACCOUNTS, pAccount->getValue(), accountInfo);
		//gAVSProxy->m_accountids.insert(sAccountId, accountInfo);
		//gAVSProxy->m_accounts.insert(pAccount->getValue(), accountInfo);
	}

	gAVSProxy->loadAccountInfo(accountInfo);

	/////////////////////////////////
	// Response
	response->setParameter(cgcParameter::create(_T("AccountId"), sAccountId));
	response->setParameter(cgcParameter::create(_T("Account"), accountInfo->getUserinfo()->getAccount()));
	response->setParameter(cgcParameter::create(_T("Name"), accountInfo->getUserinfo()->getUserName()));
	response->setParameter(cgcParameter::create(_T("Nick"), accountInfo->getUserinfo()->getNick()));
	response->setParameter(cgcParameter::create(_T("Gender"), (int)accountInfo->getUserinfo()->getGender()));
	response->setParameter(cgcParameter::create(_T("Ext"), accountInfo->getUserinfo()->getExtension()));
	response->setParameter(cgcParameter::create(_T("Phone"), accountInfo->getUserinfo()->getPhone()));
	response->setParameter(cgcParameter::create(_T("Mobile"), accountInfo->getUserinfo()->getMobile()));
	response->setParameter(cgcParameter::create(_T("Email"), accountInfo->getUserinfo()->getEmail()));
	//response->setParameter(cgcParameter::create(_T("Desc"), accountInfo->getUserinfo()->getDescription()));
	return 0;
}

extern "C" int CGC_API AccRegConfirm(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	/////////////////////////////////
	// Request
	const tstring & sAccountId = request->getParameterValue(_T("AccountId"), _T(""));
	if (sAccountId.empty()) return -1;

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
	accountInfo->getUserinfo()->setLineState(CUserInfo::UserOnLineState);

	response->sendResponse();

	// Load the offline events.
	COfflineEvent::pointer offlineEvent;
	while (accountInfo->getUserinfo()->m_offevents.front(offlineEvent))
	{
		switch (offlineEvent->getEvent())
		{
		case 601:
			{
				CUserInfoPointer toUserInfo = offlineEvent->toAccount();
				CDialogInfo::pointer dialogInfo = offlineEvent->fromInfo()->fromDialog();
				BOOST_ASSERT (toUserInfo.get() != NULL);
				BOOST_ASSERT (dialogInfo.get() != NULL);

				sendDiaInvite(response, offlineEvent->getFromAccount(), dialogInfo, toUserInfo);
			}break;
		case 602:
			{
				CDialogInfo::pointer dialogInfo = offlineEvent->fromInfo()->fromDialog();
				BOOST_ASSERT (dialogInfo.get() != NULL);

				sendDiaQuit(response, offlineEvent->getFromAccount(), dialogInfo);
			}break;
		case 302:
			{
				// FriResponse offline event.
				bool bAccept = offlineEvent->getMessage()->tostring() == _T("1");
				if (bAccept)
				{
					//CFriendInfo::pointer friendInfo;
					//if (accountInfo->m_allfriends.find(offlineEvent->getFromAccount()->getAccount(), friendInfo))
					//{
					//	friendInfo->substate(1);
					//}
				}else
				{
					//accountInfo->m_allfriends.remove(offlineEvent->getFromAccount()->getAccount());
				}
				continue;
			}break;
		case 303:
			{
				// FriDelete offline event.
				//CFriendInfo::pointer friendInfo;
				//if (accountInfo->m_allfriends.find(offlineEvent->getFromAccount()->getAccount(), friendInfo, false))
				//{
				//	friendInfo->substate(3);
				//}
				continue;
			}break;
		default:
			{
				sendOfflineEvent(response, offlineEvent);
			}break;
		}

		gAVSProxy->deleteDbOffEvent(offlineEvent->getId());
		
#ifdef WIN32
		Sleep(2);
#else
		usleep(2000);
#endif
	}
	
	// 201 online event
	/*
	CLockMap<tstring, CFriendInfo::pointer>::iterator iterFriendInfo;
	boost::mutex::scoped_lock lockFriendInfo(accountInfo->m_allfriends.mutex());
	for (iterFriendInfo=accountInfo->m_allfriends.begin(); iterFriendInfo!=accountInfo->m_allfriends.end(); iterFriendInfo++)
	{
		CFriendInfo::pointer friendInfo = iterFriendInfo->second;

		// Not subscript state.
		if (friendInfo->substate() != 1)
			continue;

		// Not register.
		CAccountInfo::pointer friendAccountInfo = (const CAccountInfo::pointer&)gApplication->getAttribute(BMT_ACCOUNTS, friendInfo->userinfo()->getAccount());
		if (friendAccountInfo.get() == NULL)
		//if (!gAVSProxy->m_accounts.find(friendInfo->userinfo()->getAccount(), friendAccountInfo))
		{
			continue;
		}

		sendUserOnOfflineEvent(friendAccountInfo, accountInfo->getUserinfo(), true);

#ifdef WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}
	lockFriendInfo.unlock();
	*/

	// 201 online event
	// company cogroup user.
	StringPointerMapPointer stringPointerMapPointer = gApplication->getStringAttributes(BMT_ACCOUNTIDS, false);
	if (stringPointerMapPointer.get() != NULL)
	{
		boost::mutex::scoped_lock lock(stringPointerMapPointer->mutex());
		CPointerMap<tstring>::iterator iter;
		for (iter=stringPointerMapPointer->begin(); iter!=stringPointerMapPointer->end(); iter++)
		{
			CAccountInfo::pointer friendAccountInfo = CGC_POINTER_CAST<CAccountInfo>(iter->second);

			if (friendAccountInfo->getUserinfo()->getAccount() != accountInfo->getUserinfo()->getAccount())
			{
				sendUserOnOfflineEvent(friendAccountInfo, accountInfo->getUserinfo(), true);
			}
#ifdef WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
		}
	}

	/////////////////////////////////
	// Response
	return 0;
}

extern "C" int CGC_API AccUnRegister(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	/////////////////////////////////
	// Request
	const tstring & sAccountId = request->getParameterValue(_T("AccountId"), _T(""));
	if (sAccountId.empty()) return -1;

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

	accountInfo->getUserinfo()->setLineState(CUserInfo::UserOffLineState);
	gApplication->removeAttribute(BMT_ACCOUNTIDS, sAccountId);
	gApplication->removeAttribute(BMT_ACCOUNTS, accountInfo->getUserinfo()->getAccount());
	//gAVSProxy->m_accountids.remove(sAccountId);
	//gAVSProxy->m_accounts.remove(accountInfo->getUserinfo()->getAccount());

	// 202 offline event
	/*
	CLockMap<tstring, CFriendInfo::pointer>::iterator iterFriendInfo;
	boost::mutex::scoped_lock lockFriendInfo(accountInfo->m_allfriends.mutex());
	for (iterFriendInfo=accountInfo->m_allfriends.begin(); iterFriendInfo!=accountInfo->m_allfriends.end(); iterFriendInfo++)
	{
		CFriendInfo::pointer friendInfo = iterFriendInfo->second;

		// Not subscript state.
		if (friendInfo->substate() != 1)
			continue;

		// Not register.
		CAccountInfo::pointer friendAccountInfo = (const CAccountInfo::pointer&)gApplication->getAttribute(BMT_ACCOUNTS, friendInfo->userinfo()->getAccount());
		if (friendAccountInfo.get() == NULL)
		//if (!gAVSProxy->m_accounts.find(friendInfo->userinfo()->getAccount(), friendAccountInfo))
		{
			continue;
		}

		sendUserOnOfflineEvent(friendAccountInfo, accountInfo->getUserinfo(), false);
#ifdef WIN32
		Sleep(5);
#else
		usleep(5000);
#endif
	}
	lockFriendInfo.unlock();
	*/

	// 202 offline event
	// company cogroup user.
	StringPointerMapPointer stringPointerMapPointer = gApplication->getStringAttributes(BMT_ACCOUNTIDS, false);
	if (stringPointerMapPointer.get() != NULL)
	{
		boost::mutex::scoped_lock lock(stringPointerMapPointer->mutex());
		CPointerMap<tstring>::iterator iter;
		for (iter=stringPointerMapPointer->begin(); iter!=stringPointerMapPointer->end(); iter++)
		{
			CAccountInfo::pointer friendAccountInfo = CGC_POINTER_CAST<CAccountInfo>(iter->second);

			sendUserOnOfflineEvent(friendAccountInfo, accountInfo->getUserinfo(), false);
#ifdef WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
		}
	}

	/////////////////////////////////
	// Response
	return 0;
}

extern "C" int CGC_API AccLoad(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	// Request
	const tstring & sAccountId = request->getParameterValue(_T("AccountId"), _T(""));
	if (sAccountId.empty()) return -1;
	//long nLoadData = request->getParameterValue("LoadData", 0);

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

	/////////////////////////////////
	// Response
	CLockMap<unsigned int, CCompanyInfoPointer>::const_iterator iterCompany;
	boost::mutex::scoped_lock lockCompanyInfo(const_cast<boost::mutex&>(accountInfo->getUserinfo()->m_companys.mutex()));
	for (iterCompany=accountInfo->getUserinfo()->m_companys.begin(); iterCompany!=accountInfo->getUserinfo()->m_companys.end(); iterCompany++)
	{
		CCompanyInfoPointer companyInfo = iterCompany->second;
		sendCompanyInfo(response, companyInfo);

#ifdef WIN32
		Sleep(5);
#else
		usleep(5000);
#endif

		CLockMap<unsigned int, CCoGroupInfo::pointer>::const_iterator iterCoGroup;
		boost::mutex::scoped_lock lockCoGroupInfo(const_cast<boost::mutex&>(companyInfo->m_allgroups.mutex()));
		for (iterCoGroup=companyInfo->m_allgroups.begin(); iterCoGroup!=companyInfo->m_allgroups.end(); iterCoGroup++)
		{
			CCoGroupInfo::pointer cogroupInfo = iterCoGroup->second;

			bool isMyCoGroup = accountInfo->m_cogroups.exist(cogroupInfo->groupid());
			sendCoGroupInfo(response, companyInfo->id(), cogroupInfo, isMyCoGroup);

#ifdef WIN32
			Sleep(2);
#else
			usleep(2000);
#endif
		}

		CLockMap<std::string, CUserInfoPointer>::iterator iterUserinfo;
		boost::mutex::scoped_lock lockUserInfo(const_cast<boost::mutex&>(companyInfo->m_userinfos.mutex()));
		for (iterUserinfo=companyInfo->m_userinfos.begin(); iterUserinfo!=companyInfo->m_userinfos.end(); iterUserinfo++)
		{
			if (iterUserinfo->second->getAccount() != accountInfo->getUserinfo()->getAccount())
			{
				sendCoGroupUserInfo(response, companyInfo->id(), 0, iterUserinfo->second);
			}

#ifdef WIN32
			Sleep(3);
#else
			usleep(3000);
#endif
		}
	}
	lockCompanyInfo.unlock();

	response->sendResponse(0, 0);
	return 0;
}

extern "C" int CGC_API AccSetPwd(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	/////////////////////////////////
	// Request
	const tstring & sAccountId = request->getParameterValue(_T("AccountId"), _T(""));
	if (sAccountId.empty()) return -1;
	const tstring & sOldPwd = request->getParameterValue(_T("OldPwd"), _T(""));
	const tstring & sNewPwd = request->getParameterValue(_T("NewPwd"), _T(""));

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
	}else if (accountInfo->getUserinfo()->getPassword() != sOldPwd)
	{
		// Password Error.
		return 13;
	}

	gAVSProxy->updatePassword(accountInfo->getUserinfo()->getAccount(), sNewPwd);
	accountInfo->getUserinfo()->setPassword(sNewPwd);

	/////////////////////////////////
	// Response
	return 0;
}

/*
extern "C" int CGC_API AccSetNick(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	/////////////////////////////////
	// Request
	const tstring & sAccountId = request->getParameterValue(_T("AccountId"), _T(""));
	if (sAccountId.empty()) return -1;
	const tstring & sNewNick = request->getParameterValue(_T("NewNick"), _T(""));

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
	if (sNewNick.empty())
		accountInfo->getUserinfo()->setNick(accountInfo->getUserinfo()->getUserName());
	else
		accountInfo->getUserinfo()->setNick(sNewNick);

	// Notify all my friends.
	CLockMap<tstring, CFriendInfo::pointer>::iterator iter;
	boost::mutex::scoped_lock lock(accountInfo->m_allfriends.mutex());
	for (iter=accountInfo->m_allfriends.begin(); iter!=accountInfo->m_allfriends.end(); iter++)
	{
		CFriendInfo::pointer friendInfo = iter->second;

		// Not subscript state.
		if (friendInfo->substate() != 1)
			continue;

		// Not register.
		CAccountInfo::pointer friendAccountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTS, friendInfo->userinfo()->getAccount()));
		if (friendAccountInfo.get() == NULL)
//		if (!gAVSProxy->m_accounts.find(friendInfo->userinfo()->getAccount(), friendAccountInfo))
		{
			continue;
		}

		cgcSession::pointer toUserCgcSession = gSystem->getcgcSession(friendAccountInfo->getSessionId());
		if (toUserCgcSession.get() == NULL) continue;
		cgcResponse::pointer toUserCgcResponse = toUserCgcSession->getLastResponse();
		if (toUserCgcResponse.get() == NULL || toUserCgcResponse->isInvalidate()) continue;

		toUserCgcResponse->setParameter(cgcParameter::create(_T("FriendAccount"), accountInfo->getUserinfo()->getAccount()));
		toUserCgcResponse->setParameter(cgcParameter::create(_T("FriendNick"), accountInfo->getUserinfo()->getNick()));
		toUserCgcResponse->sendResponse(0, 203);
#ifdef WIN32
		Sleep(20);
#else
		usleep(20000);
#endif
	}

	/////////////////////////////////
	// Response
	response->setParameter(cgcParameter::create(_T("NewNick"), accountInfo->getUserinfo()->getNick()));
	return 0;
}

*/
extern "C" int CGC_API AccSetInfo(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	// Request
	const tstring & sAccountId = request->getParameterValue(_T("AccountId"), _T(""));
	if (sAccountId.empty()) return -1;

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

	bool bModified = false;
	// UserName
	//cgcParameter::pointer userName = request->getParameter(_T("Name"));
	//if (userName.get() != 0)
	//{
	//	if (userName->getValue().empty())
	//		accountInfo->getUserinfo()->setUserName(accountInfo->getUserinfo()->getAccount());
	//	else
	//		accountInfo->getUserinfo()->setUserName(userName->getValue());
	//}
	cgcParameter::pointer userNick = request->getParameter(_T("Nick"));
	if (userNick.get() != 0)
	{
		if (userNick->getValue() != accountInfo->getUserinfo()->getNick())
		{
			bModified = true;
			accountInfo->getUserinfo()->setNick(userNick->getValue());
		}
	}

	// Gender
	int gender = request->getParameterValue(_T("Gender"), 0);
	if (gender >= 0 && gender <= 2)
	{
		if ((short)gender != accountInfo->getUserinfo()->getGender())
		{
			bModified = true;
			accountInfo->getUserinfo()->setGender((short)gender);
		}
	}

	// Birthday
	//cgcParameter::pointer birthday = request->getParameter(_T("Birthday"));
	//if (birthday.get() != 0)
	//{
	//	accountInfo->getUserinfo()->setBirthday(birthday->getValue());
	//}

	// Extension
	cgcParameter::pointer extension = request->getParameter(_T("Ext"));
	if (extension.get() != 0)
	{
		if (extension->getValue() != accountInfo->getUserinfo()->getExtension())
		{
			bModified = true;
			accountInfo->getUserinfo()->setExtension(extension->getValue());
		}
	}

	// Phone
	cgcParameter::pointer phone = request->getParameter(_T("Phone"));
	if (phone.get() != 0)
	{
		if (phone->getValue() != accountInfo->getUserinfo()->getPhone())
		{
			bModified = true;
			accountInfo->getUserinfo()->setPhone(phone->getValue());
		}
	}

	// Mobile
	cgcParameter::pointer mobile = request->getParameter(_T("Mobile"));
	if (mobile.get() != 0)
	{
		if (mobile->getValue() != accountInfo->getUserinfo()->getMobile())
		{
			bModified = true;
			accountInfo->getUserinfo()->setMobile(mobile->getValue());
		}
	}

	// Email
	cgcParameter::pointer email = request->getParameter(_T("Email"));
	if (email.get() != 0)
	{
		if (email->getValue() != accountInfo->getUserinfo()->getEmail())
		{
			bModified = true;
			accountInfo->getUserinfo()->setEmail(email->getValue());
		}
	}

	if (bModified)
	{
		gAVSProxy->updateUserinfo(accountInfo->getUserinfo());
	}

	// Response
	return 0;
}

