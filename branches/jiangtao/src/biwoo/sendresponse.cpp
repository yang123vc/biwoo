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

#ifdef WIN32
#include <windows.h>
#endif

#include "sendresponse.h"
#include <boost/filesystem.hpp>

const short MAX_PACKET_SIZE	= 1024;


bool sendDiaQuit(cgcResponse::pointer response, CUserInfo::pointer fromUser, CDialogInfo::pointer dialogInfo)
{
	BOOST_ASSERT (response.get() != 0);
	BOOST_ASSERT (fromUser.get() != 0);
	BOOST_ASSERT (dialogInfo.get() != 0);

	if (response->isInvalidate()) return false;

	response->lockResponse();
	response->setParameter(cgcParameter::create(_T("DID"), dialogInfo->dialogId()));
	response->setParameter(cgcParameter::create(_T("FromAccount"), fromUser->getAccount()));
	response->sendResponse(0, 602);
	return true;
}

bool sendDiaQuit(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, CDialogInfo::pointer dialogInfo)
{
	BOOST_ASSERT (toAccount.get() != 0);
	BOOST_ASSERT (fromUser.get() != 0);
	BOOST_ASSERT (dialogInfo.get() != 0);

	cgcSession::pointer toUserCgcSession = gSystem->getcgcSession(toAccount->getSessionId());
	if (toUserCgcSession.get() == NULL) return false;
	cgcResponse::pointer toUserCgcResponse = toUserCgcSession->getLastResponse();
	if (toUserCgcResponse.get() == NULL || toUserCgcResponse->isInvalidate()) return false;

	return sendDiaQuit(toUserCgcResponse, fromUser, dialogInfo);
}

bool sendDiaInvite(cgcResponse::pointer response, CUserInfo::pointer fromUser, CDialogInfo::pointer dialogInfo, CUserInfo::pointer inviteUserInfo)
{
	BOOST_ASSERT (response.get() != 0);
	BOOST_ASSERT (fromUser.get() != 0);
	BOOST_ASSERT (dialogInfo.get() != 0);
	BOOST_ASSERT (inviteUserInfo.get() != 0);

	if (response->isInvalidate()) return false;

	response->lockResponse();
	response->setParameter(cgcParameter::create(_T("DID"), dialogInfo->dialogId()));
	response->setParameter(cgcParameter::create(_T("FromAccount"), fromUser->getAccount()));
	response->setParameter(cgcParameter::create(_T("UA0"), inviteUserInfo->getAccount()));
	response->sendResponse(0, 601);
	return true;
}

bool sendDiaInvite(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, CDialogInfo::pointer dialogInfo, CUserInfo::pointer inviteUserInfo)
{
	BOOST_ASSERT (toAccount.get() != 0);
	BOOST_ASSERT (fromUser.get() != 0);
	BOOST_ASSERT (dialogInfo.get() != 0);
	BOOST_ASSERT (inviteUserInfo.get() != 0);

	cgcSession::pointer toUserCgcSession = gSystem->getcgcSession(toAccount->getSessionId());
	if (toUserCgcSession.get() == NULL) return false;
	cgcResponse::pointer toUserCgcResponse = toUserCgcSession->getLastResponse();
	if (toUserCgcResponse.get() == NULL || toUserCgcResponse->isInvalidate()) return false;

	return sendDiaInvite(toUserCgcResponse, fromUser, dialogInfo, inviteUserInfo);
}

bool sendDiaMembers(cgcResponse::pointer response, CUserInfo::pointer fromUser, CDialogInfo::pointer dialogInfo)
{
	BOOST_ASSERT (response.get() != 0);
	BOOST_ASSERT (fromUser.get() != 0);
	BOOST_ASSERT (dialogInfo.get() != 0);

	if (response->isInvalidate()) return false;

	response->lockResponse();
	response->setParameter(cgcParameter::create(_T("DID"), dialogInfo->dialogId()));
	response->setParameter(cgcParameter::create(_T("FromAccount"), fromUser->getAccount()));

	int index = 0;
	CLockMap<std::string, CUserInfo::pointer>::const_iterator iterUserInfo;
	boost::mutex::scoped_lock lockUserInfo(const_cast<boost::mutex&>(dialogInfo->m_members.mutex()));
	for (iterUserInfo=dialogInfo->m_members.begin(); iterUserInfo!=dialogInfo->m_members.end(); iterUserInfo++)
	{
		CUserInfo::pointer memberUserInfo = iterUserInfo->second;

		char uaBuffer[12];
		sprintf(uaBuffer, "UA%d", index++);
		response->setParameter(cgcParameter::create(uaBuffer, memberUserInfo->getAccount()));
	}

	response->sendResponse(0, 601);
	return true;
}

bool sendDiaMembers(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, CDialogInfo::pointer dialogInfo)
{
	BOOST_ASSERT (toAccount.get() != 0);
	BOOST_ASSERT (fromUser.get() != 0);
	BOOST_ASSERT (dialogInfo.get() != 0);

	cgcSession::pointer toUserCgcSession = gSystem->getcgcSession(toAccount->getSessionId());
	if (toUserCgcSession.get() == NULL) return false;
	cgcResponse::pointer toUserCgcResponse = toUserCgcSession->getLastResponse();
	if (toUserCgcResponse.get() == NULL || toUserCgcResponse->isInvalidate()) return false;

	return sendDiaMembers(toUserCgcResponse, fromUser, dialogInfo);
}

bool sendFileRequest(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, long mid, const std::string & filename, long filesize)
{
	BOOST_ASSERT (toAccount.get() != 0);
	BOOST_ASSERT (fromUser.get() != 0);

	cgcSession::pointer toUserCgcSession = gSystem->getcgcSession(toAccount->getSessionId());
	if (toUserCgcSession.get() == NULL) return false;
	cgcResponse::pointer toUserCgcResponse = toUserCgcSession->getLastResponse();
	if (toUserCgcResponse.get() == NULL || toUserCgcResponse->isInvalidate()) return false;

	toUserCgcResponse->lockResponse();
	toUserCgcResponse->setParameter(cgcParameter::create(_T("FromAccount"), fromUser->getAccount()));
	toUserCgcResponse->setParameter(cgcParameter::create(_T("Type"), 11));
	toUserCgcResponse->setParameter(cgcParameter::create(_T("MID"), mid));
	toUserCgcResponse->setParameter(cgcParameter::create(_T("Name"), filename));
	toUserCgcResponse->setParameter(cgcParameter::create(_T("Size"), filesize));
	toUserCgcResponse->sendResponse(0, 511);
	return true;
}

bool sendMsgRequest(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, long mid, long type)
{
	BOOST_ASSERT (toAccount.get() != 0);
	BOOST_ASSERT (fromUser.get() != 0);

	cgcSession::pointer toUserCgcSession = gSystem->getcgcSession(toAccount->getSessionId());
	if (toUserCgcSession.get() == NULL) return false;
	cgcResponse::pointer toUserCgcResponse = toUserCgcSession->getLastResponse();
	if (toUserCgcResponse.get() == NULL || toUserCgcResponse->isInvalidate()) return false;

	toUserCgcResponse->lockResponse();
	toUserCgcResponse->setParameter(cgcParameter::create(_T("FromAccount"), fromUser->getAccount()));
	toUserCgcResponse->setParameter(cgcParameter::create(_T("Type"), type));
	toUserCgcResponse->setParameter(cgcParameter::create(_T("MID"), mid));
	toUserCgcResponse->sendResponse(0, 511);
	return true;
}

// sendOfflineEvent
bool sendOfflineEvent(cgcResponse::pointer response, COfflineEvent::pointer offlineEvent)
{
	BOOST_ASSERT (response.get() != 0);
	BOOST_ASSERT (offlineEvent.get() != 0);

	if (response->isInvalidate()) return false;

	switch (offlineEvent->getEvent())
	{
	case 501:
		{
			CMessageInfo::pointer messageInfo = offlineEvent->getMessage();
			BOOST_ASSERT (messageInfo.get() != 0);

			long messageid = messageInfo->messageid();
			short msgtype = messageInfo->type();
			bool newflag = messageInfo->newflag();
			size_t sizeSended = 0;
			size_t tosendSize = 0;

			switch (msgtype)
			{
			case 1:
				{
					size_t toSendSizeTotal = messageInfo->total();
					cgc::cgcAttachment::pointer attach(cgcAttachment::create());
					attach->setName("text");
					attach->setTotal(toSendSizeTotal);
					while (sizeSended < toSendSizeTotal)
					{
						tosendSize = (short)(toSendSizeTotal-sizeSended) > MAX_PACKET_SIZE ? MAX_PACKET_SIZE : (toSendSizeTotal-sizeSended);
						attach->setAttach((const unsigned char*)messageInfo->getdata()+sizeSended, tosendSize);
						attach->setIndex(sizeSended);

						if (attach->getIndex() == 0)
						{
							if (!sendMsg(response, offlineEvent->getFromAccount(), offlineEvent->fromInfo(), messageid, msgtype, attach, newflag, offlineEvent->getMessage()->msgtime()))
							{
								return false;
							}
						}else
						{
							if (!sendMsg(response, offlineEvent->getFromAccount(), offlineEvent->fromInfo(), messageid, attach))
							{
								return false;
							}
						}
						sizeSended += tosendSize;

#ifdef WIN32
						Sleep(5);
#else
						usleep(5000);
#endif
					}
				}break;
			case 3:
				{
					std::string filename = messageInfo->tostring();
					size_t nFilesize = messageInfo->filesize();
					char filepath[256];
					sprintf(filepath, "%s/File/%s", gApplication->getAppConfPath().c_str(), filename.c_str());
					FILE * hfile = fopen(filepath, "rb");
					if (hfile == NULL)
					{
						return false;
					}
					unsigned char * imageBuffer = new unsigned char[nFilesize+1];
					size_t imageSize = fread(imageBuffer, 1, nFilesize, hfile);
					fclose(hfile);
					namespace fs = boost::filesystem;
					fs::path pathFile(filepath, fs::native);
					fs::remove(pathFile);

					if (imageSize == 0)
					{
						delete[] imageBuffer;
						return false;
					}

					cgc::cgcAttachment::pointer attach(cgcAttachment::create());
					attach->setName("image");
					attach->setTotal(imageSize);
					while (sizeSended < imageSize)
					{
						tosendSize = (short)(imageSize-sizeSended) > MAX_PACKET_SIZE ? MAX_PACKET_SIZE : (imageSize-sizeSended);
						attach->setAttach((const unsigned char*)imageBuffer+sizeSended, tosendSize);
						attach->setIndex(sizeSended);

						if (attach->getIndex() == 0)
						{
							long nWidth = messageInfo->imageWidth();
							long nHeight = messageInfo->imageHeight();
							if (!sendMsgImage(response, offlineEvent->getFromAccount(), offlineEvent->fromInfo(), messageid, nWidth, nHeight, msgtype, attach, newflag, offlineEvent->getMessage()->msgtime()))
							{
								delete[] imageBuffer;
								return false;
							}
						}else
						{
							if (!sendMsg(response, offlineEvent->getFromAccount(), offlineEvent->fromInfo(), messageid, attach))
							{
								delete[] imageBuffer;
								return false;
							}
						}
						sizeSended += tosendSize;

#ifdef WIN32
						Sleep(5);
#else
						usleep(5000);
#endif
					}
					delete[] imageBuffer;
				}break;
			default:
				break;
			}

		}break;
	default:
		{
			if (offlineEvent->fromInfo()->fromType() == CFromInfo::FromDialogInfo)
			{
				response->setParameter(cgcParameter::create(_T("DID"), offlineEvent->fromInfo()->fromDialog()->dialogId()));
			}
			response->setParameter(cgcParameter::create(_T("FromAccount"), offlineEvent->getFromAccount()->getAccount()));
			response->sendResponse(0, offlineEvent->getEvent());
		}break;
	}
	return true;
}

bool sendUserOnOfflineEvent(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, bool online, bool quit)
{
	BOOST_ASSERT (toAccount.get() != 0);
	BOOST_ASSERT (fromUser.get() != 0);

	cgcSession::pointer toUserCgcSession = gSystem->getcgcSession(toAccount->getSessionId());
	if (toUserCgcSession.get() == NULL) return false;
	cgcResponse::pointer toUserCgcResponse = toUserCgcSession->getLastResponse();
	if (toUserCgcResponse.get() == NULL || toUserCgcResponse->isInvalidate()) return false;

	toUserCgcResponse->lockResponse();
	toUserCgcResponse->setParameter(cgcParameter::create(_T("FromAccount"), fromUser->getAccount()));
	if (quit)
	{
		toUserCgcResponse->setParameter(cgcParameter::create2(_T("Quit"), true));
	}

	if (online)
		toUserCgcResponse->sendResponse(0, 201);
	else
		toUserCgcResponse->sendResponse(0, 202);
	return true;
}

// sendDataGroupInfo
bool sendDataGroupInfo(cgcResponse::pointer response, CDataGroup::pointer dataGroup)
{
	BOOST_ASSERT (response.get() != 0);
	BOOST_ASSERT (dataGroup.get() != 0);

	if (response->isInvalidate()) return false;

	response->lockResponse();
	response->setParameter(cgcParameter::create(_T("GroupId"), dataGroup->groupid()));
	response->setParameter(cgcParameter::create(_T("Name"), dataGroup->name()));
	if (dataGroup->parentgroup().get() != 0)
		response->setParameter(cgcParameter::create(_T("ParentId"), dataGroup->parentgroup()->groupid()));
	response->sendResponse(0, 151);
	return true;
}

// sendDataInfo
bool sendDataInfo(cgcResponse::pointer response, CDataInfo::pointer dataInfo)
{
	BOOST_ASSERT (response.get() != 0);
	BOOST_ASSERT (dataInfo.get() != 0);
	BOOST_ASSERT (dataInfo->parentgroup() != 0);

	if (response->isInvalidate()) return false;

	response->lockResponse();
	response->setParameter(cgcParameter::create(_T("GrouId"), dataInfo->parentgroup()->groupid()));
	response->setParameter(cgcParameter::create(_T("DataId"), dataInfo->dataid()));
	response->setParameter(cgcParameter::create(_T("Name"), dataInfo->name()));
	response->setParameter(cgcParameter::create(_T("Desc"), dataInfo->desc()));
	// ???
	//response->setParameter(cgcParameter::create(_T("AttachName"), dataId));
	response->sendResponse(0, 152);
	return true;
}

bool sendCompanyInfo(cgcResponse::pointer response, CCompanyInfo::pointer companyInfo)
{
	BOOST_ASSERT (response.get() != 0);
	BOOST_ASSERT (companyInfo.get() != 0);

	if (response->isInvalidate()) return false;

	response->lockResponse();
	response->setParameter(cgcParameter::create(_T("Id"), companyInfo->id()));
	response->setParameter(cgcParameter::create(_T("Name"), companyInfo->name()));
	response->sendResponse(0, 110);
	return true;
}

bool sendCoGroupInfo(cgcResponse::pointer response, unsigned int coId, CCoGroupInfo::pointer cogroupInfo, bool isMyCoGroup)
{
	BOOST_ASSERT (response.get() != 0);
	BOOST_ASSERT (cogroupInfo.get() != 0);

	if (response->isInvalidate()) return false;

	response->lockResponse();
	response->setParameter(cgcParameter::create(_T("CoId"), (long)coId));
	response->setParameter(cgcParameter::create(_T("Id"), cogroupInfo->groupid()));
	response->setParameter(cgcParameter::create(_T("Type"), (long)cogroupInfo->type()));
	response->setParameter(cgcParameter::create(_T("Name"), cogroupInfo->name()));
	if (cogroupInfo->parentgroup().get() != 0)
		response->setParameter(cgcParameter::create(_T("ParentId"), cogroupInfo->parentgroup()->groupid()));
	response->setParameter(cgcParameter::create2(_T("MyCoGroup"), isMyCoGroup));
	response->sendResponse(0, 111);
	return true;
}

bool sendCoGroupUserInfo(cgcResponse::pointer response, unsigned int coId, unsigned cogroupId, CUserInfoPointer userInfo)
{
	BOOST_ASSERT (response.get() != 0);
	BOOST_ASSERT (userInfo.get() != 0);

	if (response->isInvalidate()) return false;

	response->lockResponse();
	response->setParameter(cgcParameter::create(_T("CoId"), (long)coId));
	//response->setParameter(cgcParameter::create(_T("Id"), cogroupId));

	int index = 0;
	CLockMap<unsigned int, CCoGroupInfo::pointer>::const_iterator iterCoGroup;
	boost::mutex::scoped_lock lockCoGroupInfo(const_cast<boost::mutex&>(userInfo->m_cogroups.mutex()));
	for (iterCoGroup=userInfo->m_cogroups.begin(); iterCoGroup!=userInfo->m_cogroups.end(); iterCoGroup++)
	{
		CCoGroupInfo::pointer cogroupInfo = iterCoGroup->second;

		char buffer[10];
		sprintf(buffer, "Id%d", index++);
		response->setParameter(cgcParameter::create(buffer, (long)cogroupInfo->groupid()));
	}

	response->setParameter(cgcParameter::create(_T("Account"), userInfo->getAccount()));
	response->setParameter(cgcParameter::create(_T("Name"), userInfo->getUserName()));
	response->setParameter(cgcParameter::create(_T("Nick"), userInfo->getNick()));
	response->setParameter(cgcParameter::create(_T("Phone"), userInfo->getPhone()));
	response->setParameter(cgcParameter::create(_T("Email"), userInfo->getEmail()));
	response->setParameter(cgcParameter::create(_T("LineState"), (long)userInfo->getLineState()));
	response->sendResponse(0, 112);
	return true;
}

// sendMsg
//bool sendMsg(cgcResponse::pointer response, CUserInfo::pointer fromUser, long mid, long type, const char * msg, size_t size, bool bNew)
//{
//	BOOST_ASSERT (response.get() != 0);
//	BOOST_ASSERT (fromUser.get() != 0);
//
//	if (response->isInvalidate()) return false;
//
//	response->setParameter(cgcParameter::create(_T("FromAccount"), fromUser->getAccount()));
//	response->setParameter(cgcParameter::create2(_T("MID"), mid));
//	response->setParameter(cgcParameter::create(_T("Type"), type));
//	response->setParameter(cgcParameter::create2(_T("New"), bNew));
//	switch (type)
//	{
//	case 1:
//		{
//			std::string message(msg, size);
//			response->setParameter(cgcParameter::create(_T("Msg"), message));
//		}break;
//	default:
//		break;
//	}
//
//	response->sendResponse(0, 501);
//	return true;
//}

bool sendMsg(cgcResponse::pointer response, CUserInfo::pointer fromUser, CFromInfo::pointer fromInfo, long mid, long type, cgcAttachment::pointer attachment, bool bNew, time_t offeventTime)
{
	BOOST_ASSERT (response.get() != 0);
	BOOST_ASSERT (fromUser.get() != 0);
	BOOST_ASSERT (fromInfo.get() != 0);
	BOOST_ASSERT (attachment.get() != 0);

	if (response->isInvalidate()) return false;

	//if (fromInfo->fromType() == CFromInfo::FromDialogInfo && fromInfo->fromDialog()->history())
	//{
	//	sendDiaMembers(response, fromUser, fromInfo->fromDialog());
	//}

	response->lockResponse();
	if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
	{
		response->setParameter(cgcParameter::create(_T("DID"), fromInfo->fromDialog()->dialogId()));
	}
	response->setParameter(cgcParameter::create(_T("FromAccount"), fromUser->getAccount()));
	response->setParameter(cgcParameter::create(_T("MID"), mid));
	response->setParameter(cgcParameter::create(_T("Type"), type));
	response->setParameter(cgcParameter::create2(_T("New"), bNew));
	if (offeventTime > 0)
	{
		char buffer[20];
		sprintf(buffer, "%ld", offeventTime);
		response->setParameter(cgcParameter::create(cgcParameter::PT_DATETIME, _T("Time"), buffer));
	}
	response->setAttachInfo(attachment->getTotal(), attachment->getIndex());
	response->setAttachData(attachment->getAttachData(), attachment->getAttachSize());
	response->sendResponse(0, 501);
	return true;
}

bool sendMsg(cgcResponse::pointer response, CUserInfo::pointer fromUser, CFromInfo::pointer fromInfo, long mid, cgcAttachment::pointer attachment)
{
	BOOST_ASSERT (response.get() != 0);
	BOOST_ASSERT (fromUser.get() != 0);
	BOOST_ASSERT (fromInfo.get() != 0);
	BOOST_ASSERT (attachment.get() != 0);

	if (response->isInvalidate()) return false;

	response->lockResponse();
	if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
	{
		response->setParameter(cgcParameter::create(_T("DID"), fromInfo->fromDialog()->dialogId()));
	}
	response->setParameter(cgcParameter::create(_T("FromAccount"), fromUser->getAccount()));
	response->setParameter(cgcParameter::create(_T("MID"), mid));
	response->setAttachInfo(attachment->getTotal(), attachment->getIndex());
	response->setAttachData(attachment->getAttachData(), attachment->getAttachSize());
	response->sendResponse(0, 501);
	return true;
}

bool sendMsgImage(cgcResponse::pointer response, CUserInfo::pointer fromUser, CFromInfo::pointer fromInfo, long mid, long width, long height, long type, cgcAttachment::pointer attachment, bool bNew, time_t offeventTime)
{
	BOOST_ASSERT (response.get() != 0);
	BOOST_ASSERT (fromUser.get() != 0);
	BOOST_ASSERT (fromInfo.get() != 0);
	BOOST_ASSERT (attachment.get() != 0);

	if (response->isInvalidate()) return false;

	response->lockResponse();
	if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
	{
		response->setParameter(cgcParameter::create(_T("DID"), fromInfo->fromDialog()->dialogId()));
	}
	response->setParameter(cgcParameter::create(_T("FromAccount"), fromUser->getAccount()));
	response->setParameter(cgcParameter::create(_T("MID"), mid));
	response->setParameter(cgcParameter::create(_T("Type"), type));
	response->setParameter(cgcParameter::create(_T("Width"), width));
	response->setParameter(cgcParameter::create(_T("Height"), height));
	response->setParameter(cgcParameter::create2(_T("New"), bNew));
	if (offeventTime > 0)
	{
		char buffer[20];
		sprintf(buffer, "%ld", offeventTime);
		response->setParameter(cgcParameter::create(cgcParameter::PT_DATETIME, _T("Time"), buffer));
	}
	response->setAttachInfo(attachment->getTotal(), attachment->getIndex());
	response->setAttachData(attachment->getAttachData(), attachment->getAttachSize());
	response->sendResponse(0, 501);
	return true;
}

//bool sendMsg(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, long mid, long type, const char * msg, size_t size, bool bNew)
//{
//	BOOST_ASSERT (fromUser.get() != 0);
//	BOOST_ASSERT (toAccount.get() != 0);
//
//	cgcSession::pointer toUserCgcSession = gSystem->getcgcSession(toAccount->getSessionId());
//	if (toUserCgcSession.get() == NULL) return false;
//	cgcResponse::pointer toUserCgcResponse = toUserCgcSession->getLastResponse();
//	if (toUserCgcResponse.get() == NULL || toUserCgcResponse->isInvalidate()) return false;
//
//	return sendMsg(toUserCgcResponse, fromUser, mid, type, msg, size, bNew);
//}

bool sendMsg(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, CFromInfo::pointer fromInfo, long mid, cgcAttachment::pointer attachment)
{
	BOOST_ASSERT (fromUser.get() != 0);
	BOOST_ASSERT (fromInfo.get() != 0);
	BOOST_ASSERT (toAccount.get() != 0);

	cgcSession::pointer toUserCgcSession = gSystem->getcgcSession(toAccount->getSessionId());
	if (toUserCgcSession.get() == NULL) return false;
	cgcResponse::pointer toUserCgcResponse = toUserCgcSession->getLastResponse();
	if (toUserCgcResponse.get() == NULL || toUserCgcResponse->isInvalidate()) return false;

	return sendMsg(toUserCgcResponse, fromUser, fromInfo, mid, attachment);
}

bool sendMsg(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, CFromInfo::pointer fromInfo, long mid, long type, cgcAttachment::pointer attachment, bool bNew)
{
	BOOST_ASSERT (fromUser.get() != 0);
	BOOST_ASSERT (fromInfo.get() != 0);
	BOOST_ASSERT (toAccount.get() != 0);

	cgcSession::pointer toUserCgcSession = gSystem->getcgcSession(toAccount->getSessionId());
	if (toUserCgcSession.get() == NULL) return false;
	cgcResponse::pointer toUserCgcResponse = toUserCgcSession->getLastResponse();
	if (toUserCgcResponse.get() == NULL || toUserCgcResponse->isInvalidate()) return false;

	return sendMsg(toUserCgcResponse, fromUser, fromInfo, mid, type, attachment, bNew);
}

bool sendMsgImage(CAccountInfo::pointer toAccount, CUserInfo::pointer fromUser, CFromInfo::pointer fromInfo, long mid, long width, long height, long type, cgcAttachment::pointer attachment, bool bNew)
{
	BOOST_ASSERT (fromUser.get() != 0);
	BOOST_ASSERT (fromInfo.get() != 0);
	BOOST_ASSERT (toAccount.get() != 0);

	cgcSession::pointer toUserCgcSession = gSystem->getcgcSession(toAccount->getSessionId());
	if (toUserCgcSession.get() == NULL) return false;
	cgcResponse::pointer toUserCgcResponse = toUserCgcSession->getLastResponse();
	if (toUserCgcResponse.get() == NULL || toUserCgcResponse->isInvalidate()) return false;

	return sendMsgImage(toUserCgcResponse, fromUser, fromInfo, mid, width, height, type, attachment, bNew);
}

bool getUserInfo(CAccountInfo::pointer accountInfo, const std::string & userAccount, CUserInfo::pointer & resultUserInfo)
{
	BOOST_ASSERT (accountInfo.get() != 0);

	CFriendInfo::pointer friendInfo;
	if (accountInfo->m_allfriends.find(userAccount, friendInfo))
	{
		resultUserInfo = friendInfo->userinfo();
	}else
	{
		CLockMap<unsigned int, CCompanyInfoPointer>::const_iterator iterCompany;
		boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(accountInfo->getUserinfo()->m_companys.mutex()));
		for (iterCompany=accountInfo->getUserinfo()->m_companys.begin(); iterCompany!=accountInfo->getUserinfo()->m_companys.end(); iterCompany++)
		{
			resultUserInfo = iterCompany->second->getUserInfo(userAccount);
			if (resultUserInfo.get() != 0)
				break;
		}

		if (resultUserInfo.get() == 0)
		{
			// Friend not exist.
			return false;
		}
	}

	return true;
}
