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

// MessageMgr.cpp : Defines the initialization routines for the DLL.
//

// cgcÍ·ÎÄ¼þ
#include <CGCBase/includeapp.h>
using namespace cgc;

///////////////////////////////
// 
#include "AVSProxy.h"
#include "sendresponse.h"

long gCurrentMessageId = 0;
int gFileId = 0;

int sendtoMsg(const cgcRequest::pointer & request, cgcAttachment::pointer attach, CAccountInfo::pointer fromAccount, CFromInfo::pointer fromInfo, CUserInfo::pointer sendtoUserInfo, long mid)
{
	BOOST_ASSERT (attach.get() != NULL);
	BOOST_ASSERT (fromAccount.get() != NULL);
	BOOST_ASSERT (fromInfo.get() != NULL);
	BOOST_ASSERT (sendtoUserInfo.get() != NULL);

	CMessageInfo::pointer offlineMessageInfo = CGC_POINTER_CAST<CMessageInfo>(gApplication->getAttribute(sendtoUserInfo->getAccount(), mid));
	CAccountInfo::pointer friendAccountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTS, sendtoUserInfo->getAccount()));
	if (friendAccountInfo.get() == NULL || offlineMessageInfo.get() != NULL)
	{
		// Friend is offline state.
		if (offlineMessageInfo.get() != NULL && offlineMessageInfo->total() != attach->getTotal())
		{
			gApplication->removeAttribute(sendtoUserInfo->getAccount(), mid);
			offlineMessageInfo.reset();
		}

		if (offlineMessageInfo.get() == NULL)
		{
			if (attach->getIndex() == 0)
			{
				long type = request->getParameterValue(_T("Type"), 0);
				bool bNew = request->getParameterValue2(_T("New"), true);
				offlineMessageInfo = CMessageInfo::create(mid, attach->getTotal(), (short)type, bNew);

				if (type == 3)
				{
					long imageWidth = request->getParameterValue(_T("Width"), 0);
					long imageHeight = request->getParameterValue(_T("Height"), 0);
					offlineMessageInfo->imageWH(imageWidth, imageHeight);
				}
			}else
			{
				offlineMessageInfo = CMessageInfo::create(mid, attach->getTotal());
			}

			if (attach->getIndex() + attach->getAttachSize() == attach->getTotal())
			{
				if (attach->getIndex() > 0)
					offlineMessageInfo->m_indexs.insert(attach->getIndex(), true);
				offlineMessageInfo->m_indexs.insert(attach->getTotal(), true);
			}else
			{
				int indexCount = attach->getTotal() / attach->getAttachSize();
				if (attach->getTotal() % attach->getAttachSize() > 0)
					indexCount += 1;

				for (int i=1; i<= indexCount; i++)
				{
					unsigned long index = (i==indexCount) ? attach->getTotal() : i*attach->getAttachSize();
					offlineMessageInfo->m_indexs.insert(index, true);
				}
			}

			gApplication->setAttribute(sendtoUserInfo->getAccount(), mid, offlineMessageInfo);
		}else if (attach->getIndex() == 0)
		{
			long type = request->getParameterValue(_T("Type"), 0);
			bool bNew = request->getParameterValue2(_T("New"), true);

			offlineMessageInfo->type((short)type);
			offlineMessageInfo->newflag(bNew);
			if (type == 3)
			{
				long imageWidth = request->getParameterValue(_T("Width"), 0);
				long imageHeight = request->getParameterValue(_T("Height"), 0);
				offlineMessageInfo->imageWH(imageWidth, imageHeight);
			}
		}
		offlineMessageInfo->setdata((const char*)attach->getAttachData(), attach->getAttachSize(), attach->getIndex());
		offlineMessageInfo->m_indexs.remove(attach->getIndex() + attach->getAttachSize());
		if (!offlineMessageInfo->m_indexs.empty())
		{
			// Already had data.
			return 1;
		}

		gApplication->removeAttribute(sendtoUserInfo->getAccount(), mid);

		switch (offlineMessageInfo->type())
		{
		case 3:
			{
				// Save the image.
				char * filename = new char[50];
#ifdef WIN32
				sprintf(filename, "%I64d%08ld%d", time(0), mid, ++gFileId);
#else
				sprintf(filename, "%ld%08ld%d", time(0), mid, ++gFileId);
#endif
				char filepath[256];
				sprintf(filepath, "%s/File/%s", gApplication->getAppConfPath().c_str(), filename);
				FILE * hfile = fopen(filepath, "wb");
				if (hfile == NULL)
				{
					// System error.
					delete[] filename;
					return -2;
				}
				fwrite(offlineMessageInfo->getdata(), 1, offlineMessageInfo->total(), hfile);
				fclose(hfile);

				offlineMessageInfo->filesize(offlineMessageInfo->total());
				offlineMessageInfo->setdata(filename, strlen(filename));
			}break;
		default:
			break;
		}

		COfflineEvent::pointer offlineEvent = COfflineEvent::create(501, fromInfo, fromAccount->getUserinfo(), offlineMessageInfo);
		gAVSProxy->addOffEvent(sendtoUserInfo, offlineEvent);
		return 2;
	}

	bool ret = false;
	if (attach->getIndex() == 0)
	{
		long nType = request->getParameterValue(_T("Type"), 0);
		bool bNew = request->getParameterValue2(_T("New"), true);

		if (nType == 3)
		{
			long nWidth = request->getParameterValue(_T("Width"), 0);
			long nHeight = request->getParameterValue(_T("Height"), 0);
			ret = sendMsgImage(friendAccountInfo, fromAccount->getUserinfo(), fromInfo, mid, nWidth, nHeight, nType, attach, bNew);
		}else
		{
			ret = sendMsg(friendAccountInfo, fromAccount->getUserinfo(), fromInfo, mid, nType, attach, bNew);
		}
	}else
	{
		ret = sendMsg(friendAccountInfo, fromAccount->getUserinfo(), fromInfo, mid, attach);
	}

	return 0;
}

extern "C" int CGC_API MsgSend(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	/////////////////////////////////
	// Request
	const tstring & sAccountId = request->getParameterValue(_T("AccountId"), _T(""));
	if (sAccountId.empty()) return -1;
	long mid = request->getParameterValue(_T("MID"), 0);
	long dialogId = request->getParameterValue(_T("DID"), 0);
	if (!request->isHasAttachInfo()) return -1;

	/////////////////////////////////
	// Process
	CAccountInfo::pointer accountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTIDS, sAccountId));
	if (accountInfo.get() == NULL)
	{
		// Un register.
		return 14;
	}/*else if (session->getId().compare(accountInfo->getSessionId()) != 0)
	{
		// SessionId Error
		return -3;
	}*/
	
	if (gAVSProxy->m_rejects.exist(mid))
	{
		return 52;
	}

	cgcAttachment::pointer attach = request->getAttachment();
	BOOST_ASSERT (attach.get() != NULL);

	CDialogInfo::pointer sendtoDialogInfo;
	CUserInfo::pointer sendtoUserInfo;
	if (dialogId == 0)
	{
		const tstring & sFriendAccount = request->getParameterValue(_T("SendTo"), _T(""));
		if (sFriendAccount.empty()) return -1;

		if (!getUserInfo(accountInfo, sFriendAccount, sendtoUserInfo))
		{
			// Friend not exist.
			return 16;
		}

		return sendtoMsg(request, attach, accountInfo, CFromInfo::create(accountInfo->getUserinfo()), sendtoUserInfo, mid);
	}else
	{
		sendtoDialogInfo = CGC_POINTER_CAST<CDialogInfo>(gApplication->getAttribute(BMT_DIALOGS, dialogId));
		if (sendtoDialogInfo.get() == NULL)
		{
			// Dialog id not exist.
			return 71;
		}

		CFromInfo::pointer fromInfo = CFromInfo::create(sendtoDialogInfo);

		CLockMap<std::string, CUserInfoPointer>::iterator iter;
		for (iter=sendtoDialogInfo->m_members.begin(); iter!=sendtoDialogInfo->m_members.end(); iter++)
		{
			if (iter->second->getAccount() == accountInfo->getUserinfo()->getAccount())
			{
				continue;
			}

			int ret = sendtoMsg(request, attach, accountInfo, fromInfo, iter->second, mid);
			//if (ret != 0)
			//{
			//	return ret;
			//}
		}
	}

	// Response
	return 0;
}

extern "C" int CGC_API MsgRequest(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	/////////////////////////////////
	// Request
	const tstring & sAccountId = request->getParameterValue(_T("AccountId"), _T(""));
	if (sAccountId.empty()) return -1;
	long mid = request->getParameterValue(_T("MID"), 0);
	const tstring & sFriendAccount = request->getParameterValue(_T("SendTo"), _T(""));
	if (sFriendAccount.empty()) return -1;
	long type = request->getParameterValue(_T("Type"), 0);

	/////////////////////////////////
	// Process
	CAccountInfo::pointer accountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTIDS, sAccountId));
	if (accountInfo.get() == NULL)
//	if (!gAVSProxy->m_accountids.find(sAccountId, accountInfo))
	{
		// Un register.
		return 14;
	}else if (session->getId().compare(accountInfo->getSessionId()) != 0)
	{
		// SessionId Error
		return -3;
	}

	CAccountInfo::pointer friendAccountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTS, sFriendAccount));
	if (friendAccountInfo.get() == NULL)
	//if (!gAVSProxy->m_accounts.find(sFriendAccount, friendAccountInfo))
	{
		// SentTo Account offline state;
		return 17;
	}

	long newmid = ++gCurrentMessageId;

	if (type == 11)
	{
		const tstring & filename = request->getParameterValue(_T("Name"), _T(""));
		if (filename.empty()) return -1;
		long filesize = request->getParameterValue(_T("Size"), 0);
		if (filesize <= 0) return -1;

		response->lockResponse();
		response->setParameter(cgcParameter::create(_T("MID"), mid));
		response->setParameter(cgcParameter::create(_T("NMID"), newmid));
		response->sendResponse();

		sendFileRequest(friendAccountInfo, accountInfo->getUserinfo(), newmid, filename, filesize);
	}else// if ()
	{
		response->lockResponse();
		response->setParameter(cgcParameter::create(_T("MID"), mid));
		response->setParameter(cgcParameter::create(_T("NMID"), newmid));
		response->sendResponse();

		sendMsgRequest(friendAccountInfo, accountInfo->getUserinfo(), newmid, type);
	}

	// Response
	return 0;
}

extern "C" int CGC_API MsgResponse(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	/////////////////////////////////
	// Request
	const tstring & sAccountId = request->getParameterValue(_T("AccountId"), _T(""));
	if (sAccountId.empty()) return -1;
	long mid = request->getParameterValue(_T("MID"), 0);
	const tstring & sFriendAccount = request->getParameterValue(_T("SendTo"), _T(""));
	if (sFriendAccount.empty()) return -1;
	long nResponse = request->getParameterValue(_T("Response"), 0);

	/////////////////////////////////
	// Process
	CAccountInfo::pointer accountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTIDS, sAccountId));
	if (accountInfo.get() == NULL)
//	if (!gAVSProxy->m_accountids.find(sAccountId, accountInfo))
	{
		// Un register.
		return 14;
	}else if (session->getId().compare(accountInfo->getSessionId()) != 0)
	{
		// SessionId Error
		return -3;
	}

	if (nResponse == 1)
		gAVSProxy->m_rejects.remove(mid);
	else
		gAVSProxy->m_rejects.insert(mid, true);

	CAccountInfo::pointer friendAccountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTS, sFriendAccount));
	if (friendAccountInfo.get() == NULL)
//	if (!gAVSProxy->m_accounts.find(sFriendAccount, friendAccountInfo))
	{
		// SentTo Account offline state;
		return 17;
	}

	cgcSession::pointer toUserCgcSession = gSystem->getcgcSession(friendAccountInfo->getSessionId());
	if (toUserCgcSession.get() == NULL) return false;
	cgcResponse::pointer toUserCgcResponse = toUserCgcSession->getLastResponse();
	if (toUserCgcResponse.get() == NULL || toUserCgcResponse->isInvalidate()) return false;

	toUserCgcResponse->lockResponse();
	toUserCgcResponse->setParameter(cgcParameter::create(_T("FromAccount"), accountInfo->getUserinfo()->getAccount()));
	toUserCgcResponse->setParameter(cgcParameter::create(_T("MID"), mid));
	toUserCgcResponse->setParameter(cgcParameter::create(_T("Response"), nResponse));
	toUserCgcResponse->sendResponse(0, 512);

	return 0;
}

extern "C" int CGC_API MsgData(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	/////////////////////////////////
	// Request
	const tstring & sAccountId = request->getParameterValue(_T("AccountId"), _T(""));
	if (sAccountId.empty()) return -1;
	long mid = request->getParameterValue(_T("MID"), 0);
	const tstring & sFriendAccount = request->getParameterValue(_T("SendTo"), _T(""));
	if (sFriendAccount.empty()) return -1;
	if (!request->isHasAttachInfo()) return -1;

	/////////////////////////////////
	// Process
	CAccountInfo::pointer accountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTIDS, sAccountId));
	if (accountInfo.get() == NULL)
//	if (!gAVSProxy->m_accountids.find(sAccountId, accountInfo))
	{
		// Un register.
		return 14;
	}else if (session->getId().compare(accountInfo->getSessionId()) != 0)
	{
		// SessionId Error
		return -3;
	}

	CUserInfo::pointer sendtoUserInfo;
	if (!getUserInfo(accountInfo, sFriendAccount, sendtoUserInfo))
	{
		// Friend not exist.
		return 16;
	}

	cgcAttachment::pointer attach = request->getAttachment();
	BOOST_ASSERT (attach.get() != NULL);

	CMessageInfo::pointer messageInfo = CGC_POINTER_CAST<CMessageInfo>(gApplication->getAttribute(BMT_OFFLINEMSG, mid));
	//gAVSProxy->m_offlinemsg.find(mid, messageInfo);

	CAccountInfo::pointer friendAccountInfo = CGC_POINTER_CAST<CAccountInfo>(gApplication->getAttribute(BMT_ACCOUNTS, sFriendAccount));
	if (friendAccountInfo.get() == NULL || messageInfo.get() != NULL)
//	if (!gAVSProxy->m_accounts.find(sFriendAccount, friendAccountInfo) || messageInfo.get() != NULL)
	{
		// Friend is offline state.
		if (messageInfo.get() != NULL && messageInfo->total() != attach->getTotal())
		{
			gApplication->removeAttribute(BMT_OFFLINEMSG, mid);
			//gAVSProxy->m_offlinemsg.remove(mid);
			messageInfo.reset();
		}

		if (messageInfo.get() == NULL)
		{
			if (attach->getIndex() == 0)
			{
				long type = request->getParameterValue(_T("Type"), 0);
				bool bNew = request->getParameterValue2(_T("New"), true);
				messageInfo = CMessageInfo::create(mid, attach->getTotal(), (short)type, bNew);

				if (type == 3)
				{
					long imageWidth = request->getParameterValue(_T("Width"), 0);
					long imageHeight = request->getParameterValue(_T("Height"), 0);
					messageInfo->imageWH(imageWidth, imageHeight);
				}
			}else
			{
				messageInfo = CMessageInfo::create(mid, attach->getTotal());
			}

			if (attach->getIndex() + attach->getAttachSize() == attach->getTotal())
			{
				if (attach->getIndex() > 0)
					messageInfo->m_indexs.insert(attach->getIndex(), true);
				messageInfo->m_indexs.insert(attach->getTotal(), true);
			}else
			{
				int indexCount = attach->getTotal() / attach->getAttachSize();
				if (attach->getTotal() % attach->getAttachSize() > 0)
					indexCount += 1;

				for (int i=1; i<= indexCount; i++)
				{
					unsigned long index = (i==indexCount) ? attach->getTotal() : i*attach->getAttachSize();
					messageInfo->m_indexs.insert(index, true);
				}
			}

			gApplication->setAttribute(BMT_OFFLINEMSG, mid, messageInfo);
			//gAVSProxy->m_offlinemsg.insert(mid, messageInfo);
		}else if (attach->getIndex() == 0)
		{
			long type = request->getParameterValue(_T("Type"), 0);
			bool bNew = request->getParameterValue2(_T("New"), true);

			messageInfo->type((short)type);
			messageInfo->newflag(bNew);
			if (type == 3)
			{
				long imageWidth = request->getParameterValue(_T("Width"), 0);
				long imageHeight = request->getParameterValue(_T("Height"), 0);
				messageInfo->imageWH(imageWidth, imageHeight);
			}
		}
		messageInfo->setdata((const char*)attach->getAttachData(), attach->getAttachSize(), attach->getIndex());
		messageInfo->m_indexs.remove(attach->getIndex() + attach->getAttachSize());
		if (!messageInfo->m_indexs.empty())
		{
			// Already had data.
			return 17;
		}
		gApplication->removeAttribute(BMT_OFFLINEMSG, mid);
		//gAVSProxy->m_offlinemsg.remove(mid);

		switch (messageInfo->type())
		{
		case 3:
			{
				// Save the image.
				char * filename = new char[50];
#ifdef WIN32
				sprintf(filename, "%I64d%08ld", time(0), mid);
#else
				sprintf(filename, "%ld%08ld", time(0), mid);
#endif
				char filepath[256];
				sprintf(filepath, "%s/File/%s", gApplication->getAppConfPath().c_str(), filename);
				FILE * hfile = fopen(filepath, "wb");
				if (hfile == NULL)
				{
					// System error.
					delete[] filename;
					return -2;
				}
				fwrite(messageInfo->getdata(), 1, messageInfo->total(), hfile);
				fclose(hfile);

				messageInfo->filesize(messageInfo->total());
				messageInfo->setdata(filename, strlen(filename));
			}break;
		default:
			break;
		}

		COfflineEvent::pointer offlineEvent = COfflineEvent::create(501, CFromInfo::create(accountInfo->getUserinfo()), accountInfo->getUserinfo(), messageInfo);
		gAVSProxy->addOffEvent(sendtoUserInfo, offlineEvent);
		return 17;
	}

	CDialogInfo::pointer nullDialogInfo;
	bool ret = false;
	if (attach->getIndex() == 0)
	{
		long nType = request->getParameterValue(_T("Type"), 0);
		bool bNew = request->getParameterValue2(_T("New"), true);

		if (nType == 3)
		{
			long nWidth = request->getParameterValue(_T("Width"), 0);
			long nHeight = request->getParameterValue(_T("Height"), 0);
			ret = sendMsgImage(friendAccountInfo, accountInfo->getUserinfo(), CFromInfo::create(accountInfo->getUserinfo()), mid, nWidth, nHeight, nType, attach, bNew);
		}else
		{
			ret = sendMsg(friendAccountInfo, accountInfo->getUserinfo(), CFromInfo::create(accountInfo->getUserinfo()), mid, nType, attach, bNew);
		}
	}else
	{
		ret = sendMsg(friendAccountInfo, accountInfo->getUserinfo(), CFromInfo::create(accountInfo->getUserinfo()), mid, attach);
	}

	// Response
	return 0;
}
