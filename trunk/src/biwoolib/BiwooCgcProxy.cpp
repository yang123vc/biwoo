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

#include "biwooCgcProxy.h"
#include <bodb/bodb2.h>
#include <bodb/fieldvariant.h>
using namespace bo;
#include <libCoder/libCoder.h>
#include <sys/timeb.h>

#pragma warning(disable:4819)

//const short MAX_TEXT_SIZE	= 1024;
const short MAX_PACKET_SIZE	= 1500;

//CBiwooCgcProxy::CBiwooCgcProxy(CAvsHandler * handler)
CBiwooCgcProxy::CBiwooCgcProxy(void)
: m_handler(NULL)
, m_bDoAccountUnRegister(false)
, m_cgcClient(NULL), m_fileClient(NULL)
, m_currentMID(0)
, m_LDoDSHandler(NULL), m_RDoDSHandler(NULL)

{
#ifdef WIN32
	m_sModulePath = cgcString::GetModulePath();
#else
	namespace fs = boost::filesystem;
	fs::path currentPath( fs::initial_path());
	m_sModulePath = currentPath.string();
#endif

}

CBiwooCgcProxy::~CBiwooCgcProxy(void)
{
	stop();
}

bool CBiwooCgcProxy::start(const CCgcAddress & serverAddr, const CCgcAddress & fileServerAddr, const CCgcAddress & p2pAddr, const CCgcAddress & p2pudpAddr)
{
	//std::locale::global(std::locale(""));

	m_serverAddr = serverAddr;
	m_fileServerAddr = fileServerAddr;
	m_p2pAddr = p2pAddr;
	m_p2pudpAddr = p2pudpAddr;

	m_p2pav.SetP2PAVHandler(this);
	if (!m_p2pav.connectStreamServer(m_serverAddr, m_p2pAddr, p2pudpAddr))
	{
		m_p2pav.disconnectStreamServer();
		return false;
	}
	//PRESULTSET resultset = 0;
	//bodb_exec("SELECT * FROM userinfo_t", &resultset);

	if (m_cgcClient == NULL)
		m_cgcClient = m_sotpClient.startClient(m_serverAddr);
	if (m_cgcClient == NULL)
		return false;

	if (m_fileClient == NULL)
		m_fileClient = m_sotpClient.startClient(m_fileServerAddr);
	if (m_fileClient == NULL)
		return false;

	// File Server
	m_fileClient->doStartActiveThread();
	m_fileClient->doSetResponseHandler(this);
	m_fileClient->doSetAppName(const_Avs_AppName);
	bool ret = m_fileClient->doSendOpenSession();
	if (ret)
	{
		int i=0;
		while (i++ < 20)
		{
			if (m_fileClient->doIsSessionOpened())
			{
				break;
			}
#ifdef WIN32
			Sleep(100);
#else
			usleep(100000);
#endif
		}
	}

	m_cgcClient->doStartActiveThread();
	m_cgcClient->doSetResponseHandler(this);
	m_cgcClient->doSetAppName(const_Avs_AppName);
	ret = m_cgcClient->doSendOpenSession();
	if (ret)
	{
		int i=0;
		while (i++ < 20)
		{
			if (m_cgcClient->doIsSessionOpened())
				return true;
#ifdef WIN32
			Sleep(100);
#else
			usleep(100000);
#endif
		}
	}

	return ret;
}

void CBiwooCgcProxy::stop(void)
{
	if (m_cgcClient != NULL)
	{
		m_p2pav.disconnectStreamServer();

		CLockMap<long, CSendInfo::pointer>::iterator pIterSends;
		for (pIterSends=m_sends.begin(); pIterSends!=m_sends.end(); pIterSends++)
		{
			pIterSends->second->accept(false);
		}
		m_sends.clear();
		m_recvs.clear();
		CLockMapPtr<long, boost::thread*>::iterator pIter;
		for (pIter=m_sendthreads.begin(); pIter!=m_sendthreads.end(); pIter++)
		{
			boost::thread * sendThread = pIter->second;
			sendThread->join();
		}
		m_sendthreads.clear();

		accountUnRegister();
#ifdef WIN32
		Sleep(200);
#else
		usleep(200000);
#endif
		DoSotpClientHandler * handlertemp = m_cgcClient;
		m_cgcClient = NULL;
		handlertemp->doSendCloseSession();
		m_sotpClient.stopClient(handlertemp);

		handlertemp = m_fileClient;
		m_fileClient = NULL;
		handlertemp->doSendCloseSession();
		m_sotpClient.stopClient(handlertemp);

		m_account.reset();

		m_dialogs.clear();
		m_avs.clear();
		m_avs2.clear();
		m_messageinfos.clear();
		m_rejects.clear();
		m_unreads.clear();
		m_recents.clear();
		bodb_exit();
	}
}

CAccountConversation::pointer CBiwooCgcProxy::getAccountConversations(CFromInfo::pointer fromInfo)
{
	BOOST_ASSERT (fromInfo.get() != 0);

	CAccountConversation::pointer accountConversation;
	if (m_account.get() == 0) return accountConversation;

	if (fromInfo->fromType() == CFromInfo::FromUserInfo)
	{
		if (m_avs.find(fromInfo->fromUser()->getAccount(), accountConversation))
		{
			return accountConversation;
		}
		accountConversation = CAccountConversation::create(fromInfo);
		m_avs.insert(fromInfo->fromUser()->getAccount(), accountConversation);
	}else if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
	{
		if (m_avs2.find(fromInfo->fromDialog()->dialogId(), accountConversation))
		{
			return accountConversation;
		}
		accountConversation = CAccountConversation::create(fromInfo);
		m_avs2.insert(fromInfo->fromDialog()->dialogId(), accountConversation);
	}

	PRESULTSET resultset = 0;
	char sqlBuffer[4098];
	memset(sqlBuffer, 0, sizeof(sqlBuffer));
	if (fromInfo->fromType() == CFromInfo::FromUserInfo)
	{
		sprintf(sqlBuffer, "SELECT sayaccount,msgtype,subtype,newflag,message,msgsize,width,height,msgtime FROM accountconversations_t \
						   WHERE fromtype=%d AND fromaccount='%s'", (int)CFromInfo::FromUserInfo, fromInfo->fromUser()->getAccount().c_str());
	}else if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
	{
		sprintf(sqlBuffer, "SELECT fromaccount,sayaccount,msgtype,subtype,newflag,message,msgsize,width,height,msgtime FROM accountconversations_t \
						   WHERE fromtype=%d AND fromid=%ld", (int)CFromInfo::FromDialogInfo, fromInfo->fromDialog()->dialogId());
	}else
	{
		// error
		return accountConversation;
	}

	bodb_exec(sqlBuffer, &resultset);
	if (resultset != 0)
	{
		for (int i=0; i<resultset->rscount; i++)
		{
			CUserInfo::pointer fromAccount;
			int offsetIndex = 0;
			if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
			{
				CFieldVariant varFromAccount(resultset->rsvalues[i]->fieldvalues[offsetIndex++]);
				std::string sFromAccount = varFromAccount.getString();

				if (sFromAccount == m_account->getUserinfo()->getAccount())
					fromAccount = m_account->getUserinfo();
				else
				{
					fromAccount = this->getUserInfo(sFromAccount);
					//if (fromAccount.get() == NULL)
					//{
					//	continue;
					//}
				}
			}

			CFieldVariant varSayAccount(resultset->rsvalues[i]->fieldvalues[0+offsetIndex]);
			CFieldVariant varMsgType(resultset->rsvalues[i]->fieldvalues[1+offsetIndex]);
			CFieldVariant varSubType(resultset->rsvalues[i]->fieldvalues[2+offsetIndex]);
			CFieldVariant varNewFlag(resultset->rsvalues[i]->fieldvalues[3+offsetIndex]);
			CFieldVariant varMessage(resultset->rsvalues[i]->fieldvalues[4+offsetIndex]);
			CFieldVariant varMsgSize(resultset->rsvalues[i]->fieldvalues[5+offsetIndex]);
			//if (varMsgSize.getIntu() <= 0)
			//	continue;

			CFieldVariant varMsgTime(resultset->rsvalues[i]->fieldvalues[8+offsetIndex]);

			CConversationInfo::ConversationType conversationType = (CConversationInfo::ConversationType)varSubType.getInt(1);
			std::string sSayAccount = varSayAccount.getString();
			CUserInfo::pointer sayAccount;
			if (sSayAccount == m_account->getUserinfo()->getAccount())
				sayAccount = m_account->getUserinfo();
			else
			{
				if (fromInfo->fromType() == CFromInfo::FromUserInfo)
				{
					sayAccount = fromInfo->fromUser();
				}else if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
				{
					//sSayAccount = varFromAccount->getString();
					if (sSayAccount == m_account->getUserinfo()->getAccount())
						sayAccount = m_account->getUserinfo();
					else
					{
						sayAccount = this->getUserInfo(sSayAccount);
						if (sayAccount.get() == NULL)
						{
							// error
							continue;
						}
					}
				}
			}
			//sayAccount = getUserInfo(varSayAccount.getString());
			if (sayAccount.get() != 0)
			{
				int nMsgType = varMsgType.getInt();
				//CMessageInfo::pointer messageInfo = CMessageInfo::create(0, varMsgSize.getIntu(), msgtype, varNewFlag.getBool(true));
				CMessageInfo::pointer messageInfo;
				long mid = time(0)+(++m_currentMID);
				switch (nMsgType)
				{
				case 602:
				case 601:
					{
						if (conversationType == CConversationInfo::CT_CREATE && sSayAccount == m_account->getUserinfo()->getAccount())
						{
							break;
						}

						if (fromInfo->fromType() != CFromInfo::FromDialogInfo)
						{
							// error
							break;
						}

						messageInfo = CMessageInfo::create(mid, 0, nMsgType, varNewFlag.getBool(true));
					}break;
				case 1:
				case 11:
					{
						std::string sString = varMessage.getString();
						messageInfo = CMessageInfo::create(mid, sString.size(), nMsgType, varNewFlag.getBool(true));
						messageInfo->setdata(sString.c_str(), sString.size(), 0);
						if (nMsgType == 11)
						{
							messageInfo->filesize(varMsgSize.getIntu());
						}
					}break;
				case 3:
					{
						messageInfo = CMessageInfo::create(mid, varMsgSize.getIntu(), nMsgType, varNewFlag.getBool(true));
						CFieldVariant varWidth(resultset->rsvalues[i]->fieldvalues[6+offsetIndex]);
						CFieldVariant varHeight(resultset->rsvalues[i]->fieldvalues[7+offsetIndex]);

						messageInfo->imageWH(varWidth.getInt(), varHeight.getInt());
						CAVCodecProxy m_videoCodec;
						FVInfo fvinfo(PIX_FMT_BGR24, 24, messageInfo->imageWidth(), messageInfo->imageHeight());
						m_videoCodec.setdec_info(fvinfo);
						if (!m_videoCodec.openav(CODEC_ID_MPEG4))
						{
							messageInfo.reset();
							break;
						}

						std::string sFilename = varMessage.getString();
						size_t nFilesize = varMsgSize.getIntu();
						char filepath[256];
						sprintf(filepath, "%s/users/%s/File/%s", this->m_sModulePath.c_str(), m_account->getUserinfo()->getAccount().c_str(), sFilename.c_str());
						FILE * hfile = fopen(filepath, "rb");
						if (hfile != NULL)
						{
							unsigned char * buffer = new unsigned char[nFilesize+1];
							size_t readed = fread(buffer, 1, nFilesize, hfile);
							if (readed == 0)
							{
								delete[] buffer;
								messageInfo.reset();
								fclose(hfile);
								break;
							}

							unsigned char * decode_buffer;
							int decode_size = 0;
							if (!m_videoCodec.decode(buffer, messageInfo->total(), &decode_buffer, decode_size, false))
							{
								delete[] buffer;
								messageInfo.reset();
								fclose(hfile);
								break;
							}

							messageInfo->setdata((char*)decode_buffer, decode_size);
							delete[] buffer;
							fclose(hfile);
						}
					}break;
				default:
					//messageInfo.reset();
					break;
				}
				if (messageInfo.get() != NULL)
				{
					messageInfo->msgtime(varMsgTime.getIntu());
					CConversationInfo::pointer conversationInfo = CConversationInfo::create(fromInfo, sayAccount, messageInfo, conversationType);
					conversationInfo->fromAccount(fromAccount);
					//conversationInfo->datetime(varMsgTime.getString());
					accountConversation->m_conversations.add(conversationInfo);
				}
			}
		}

		bodb_free(resultset);
	}

	if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
	{
		getDialogInfo(fromInfo->fromDialog()->dialogId());
	}

	return accountConversation;
}

CDialogInfo::pointer CBiwooCgcProxy::getDialogInfo(long dialogid)
{
	CDialogInfo::pointer dialogInfo;
	if (!m_dialogs.find(dialogid, dialogInfo))
	{
		PRESULTSET resultset = 0;
		char sqlBuffer[4098];
		memset(sqlBuffer, 0, sizeof(sqlBuffer));
		sprintf(sqlBuffer, "SELECT account,manager FROM historydialog_t WHERE dialogid=%ld", dialogid);

		bodb_exec(sqlBuffer, &resultset);
		if (resultset != 0)
		{
			for (int i=0; i<resultset->rscount; i++)
			{
				CFieldVariant varAccount(resultset->rsvalues[i]->fieldvalues[0]);
				CFieldVariant varManager(resultset->rsvalues[i]->fieldvalues[1]);

				std::string sFromAccount = varAccount.getString();
				bool bManager = varManager.getBool(false);
				CUserInfo::pointer fromAccount;
				if (sFromAccount == m_account->getUserinfo()->getAccount())
					fromAccount = m_account->getUserinfo();
				else
				{
					fromAccount = this->getUserInfo(sFromAccount);
					if (fromAccount.get() == NULL)
					{
						continue;
					}
				}

				if (dialogInfo.get() == NULL)
				{
					dialogInfo = CDialogInfo::create(dialogid, fromAccount);
					m_dialogs.insert(dialogInfo->dialogId(), dialogInfo);
				}
				if (bManager)
				{
					dialogInfo->manager(fromAccount);
				}
				if (sFromAccount != m_account->getUserinfo()->getAccount())
				{
					dialogInfo->m_members.insert(sFromAccount, fromAccount);
				}
			}
			bodb_free(resultset);
		}
	}

	return dialogInfo;
}

CConversationInfo::pointer CBiwooCgcProxy::getUnreadConversation(void)
{
	CConversationInfo::pointer result;

	if (m_unreads.front(result))
	{
		CFromInfo::pointer fromInfo = result->fromInfo();
		//CUserInfo::pointer fromUserInfo = result->sayAccount();

		CAccountConversation::pointer accountConversation;

		if (fromInfo->fromType() == CFromInfo::FromUserInfo)
		{
			if (!m_avs.find(fromInfo->fromUser()->getAccount(), accountConversation))
			{
				accountConversation = CAccountConversation::create(fromInfo);
				m_avs.insert(fromInfo->fromUser()->getAccount(), accountConversation);
			}
		}else if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
		{
			if (!m_avs2.find(fromInfo->fromDialog()->dialogId(), accountConversation))
			{
				accountConversation = CAccountConversation::create(fromInfo);
				m_avs2.insert(fromInfo->fromDialog()->dialogId(), accountConversation);
			}
		}

		accountConversation->m_conversations.add(result);
	}

	return result;
}

void CBiwooCgcProxy::deleteUserConversation(CFromInfo::pointer fromInfo)
{
	BOOST_ASSERT (fromInfo.get() != NULL);

	char sqlBuffer[1024];
	memset(sqlBuffer, 0, sizeof(sqlBuffer));
	if (fromInfo->fromType() == CFromInfo::FromUserInfo)
	{
		sprintf(sqlBuffer, "DELETE FROM accountconversations_t WHERE fromtype=%d AND fromaccount='%s'",
			(int)CFromInfo::FromUserInfo, fromInfo->fromUser()->getAccount().c_str());
		CAccountConversation::pointer accountConversation;
		if (m_avs.find(fromInfo->fromUser()->getAccount(), accountConversation))
		{
			accountConversation->m_conversations.clear();
		}
	}else if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
	{
		CDialogInfo::pointer dialogInfo = fromInfo->fromDialog();

		//
		sprintf(sqlBuffer, "DELETE FROM historydialog_t WHERE dialogid=%ld", dialogInfo->dialogId());
		bodb_exec(sqlBuffer);

		if (dialogInfo->manager()->getAccount() == m_account->getUserinfo()->getAccount())
		{
			sprintf(sqlBuffer, "INSERT INTO historydialog_t (dialogid,account,manager)\
							   VALUES(%ld,'%s',true)",
							   dialogInfo->dialogId(), m_account->getUserinfo()->getAccount().c_str());
			bodb_exec(sqlBuffer);
		}

		CLockMap<tstring, CUserInfo::pointer>::iterator iter;
		for (iter=dialogInfo->m_members.begin(); iter!=dialogInfo->m_members.end(); iter++)
		{
			CUserInfoPointer userInfo = iter->second;
			bool bManager = userInfo->getAccount() == dialogInfo->manager()->getAccount();

			sprintf(sqlBuffer, "INSERT INTO historydialog_t (dialogid,account,manager)\
							   VALUES(%ld,'%s',%s)",
							   dialogInfo->dialogId(), userInfo->getAccount().c_str(), bManager ? "true" : "false");
			bodb_exec(sqlBuffer);
		}

		//m_dialogs.remove(fromInfo->fromDialog()->dialogId());

		sprintf(sqlBuffer, "DELETE FROM accountconversations_t WHERE fromtype=%d AND fromid=%ld", 
			(int)CFromInfo::FromDialogInfo, fromInfo->fromDialog()->dialogId());
		CAccountConversation::pointer accountConversation;
		if (m_avs2.find(fromInfo->fromDialog()->dialogId(), accountConversation))
		{
			accountConversation->m_conversations.clear();
		}
	}

	bodb_exec(sqlBuffer);

}

bool CBiwooCgcProxy::createFriendGroup(CGroupInfo::pointer groupInfo)
{
	BOOST_ASSERT (groupInfo.get() != 0);

	if (!isOpenSession()) return false;
	if (m_account.get() == 0) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Name"), groupInfo->name()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Type"), (int)groupInfo->type()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("ParentGroup"), groupInfo->parentgroupid()));

	m_cgcClient->doSendAppCall(const_CallSign_FriGrpCreate, const_Api_FriGrpCreate);
	return true;
}

bool CBiwooCgcProxy::deleteFriendGroup(unsigned int groupId)
{
	if (!isOpenSession()) return false;
	if (m_account.get() == 0) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Id"), groupId));

	m_cgcClient->doSendAppCall(const_CallSign_FriGrpDestroy, const_Api_FriGrpDestroy);
	return true;
}

bool CBiwooCgcProxy::groupChangeName(long groupId, const tstring & newName)
{
	if (!isOpenSession()) return false;
	if (m_account.get() == 0) return false;

	//if (groupId <= 0 || newName.empty()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Id"), groupId));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("NewName"), newName));

	m_cgcClient->doSendAppCall(const_CallSign_FriGrpSetName, const_Api_FriGrpSetName);
	return true;
}

bool CBiwooCgcProxy::friendLoad(void)
{
	if (!isOpenSession()) return false;
	if (m_account.get() == 0) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));

	m_cgcClient->doSendAppCall(const_CallSign_FriLoad, const_Api_FriLoad);
	return true;
}

bool CBiwooCgcProxy::coLoad(void)
{
	if (!isOpenSession()) return false;
	if (m_account.get() == 0) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));

	m_cgcClient->doSendAppCall(const_CallSign_CoLoad, const_Api_CoLoad);
	return true;
}

bool CBiwooCgcProxy::coLoadGroup(unsigned int coId)
{
	if (!isOpenSession()) return false;
	if (m_account.get() == 0) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("CoId"), coId));

	m_cgcClient->doSendAppCall(const_CallSign_CoLoadGroup, const_Api_CoLoadGroup);
	return true;
}

bool CBiwooCgcProxy::coLoadGroupUser(unsigned int coId, unsigned int cogroupId)
{
	if (!isOpenSession()) return false;
	if (m_account.get() == 0) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("CoId"), coId));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("GroupId"), cogroupId));

	m_cgcClient->doSendAppCall(const_CallSign_CoLoadGroupUser, const_Api_CoLoadGroupUser);
	return true;
}

bool CBiwooCgcProxy::accountRegister(const tstring & accountName, const tstring & sPassword)
{
	if (!isOpenSession()) return false;
	if (m_account.get() != 0) return true;

	m_bDoAccountUnRegister = false;
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Account"), accountName));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Password"), sPassword));

	m_cgcClient->doSendAppCall(const_CallSign_Register, const_Api_Register);

	int i=0;
	while (i++ < 20)
	{
		if (m_account.get() != 0)
			return true;
#ifdef WIN32
		Sleep(100);
#else
		usleep(100000);
#endif
	}

	return true;
}

bool CBiwooCgcProxy::accountRegConfirm(void)
{
	if (!isOpenSession()) return false;
	if (m_account.get() == 0) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doSendAppCall(const_CallSign_RegConfirm, const_Api_RegConfirm);
	return true;
}

bool CBiwooCgcProxy::accountUnRegister(void)
{
	if (!isOpenSession()) return false;
	if (m_account.get() == 0) return false;

	m_bDoAccountUnRegister = true;
	//boost::mutex::scoped_lock lock(m_p2pProxy.mutex());
	//CLockMapPtr<tstring, P2PClientPointer>::iterator pIter;
	//for (pIter=m_p2pProxy.begin(); pIter!=m_p2pProxy.end(); pIter++)
	//{
	//	P2PClientPointer pP2PClient = pIter->second;
	//	if (pP2PClient->getLocalP2PStatus() && pP2PClient->getRemoteP2PStatus())
	//		p2pDisconnect(pP2PClient);

	//	DoSotpClientHandler * pDoHandler = pP2PClient->dohandler();
	//	BOOST_ASSERT(pDoHandler != NULL);

	//	pDoHandler->doSetRemoteAddr(m_sUdpAddr);
	//	m_sotpClient.stopClient(pDoHandler);
	//}
	//m_p2pProxy.clear(false);


	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doSendAppCall(const_CallSign_UnRegister, const_Api_UnRegister);

	return true;
}

bool CBiwooCgcProxy::accountLoad(void)
{
	if (!isOpenSession()) return false;
	if (m_account.get() == 0) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("LoadData"), 7));
	m_cgcClient->doSendAppCall(const_CallSign_AccLoad, const_Api_AccLoad);

	return true;
}

bool CBiwooCgcProxy::createConference(const tstring & conferenceName)
{
	if (!isOpenSession()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("Conference"), conferenceName));

	m_cgcClient->doSendAppCall(const_CallSign_CreateConference, const_Avs_Api_CreateConference);
	return true;
}

bool CBiwooCgcProxy::enableAudioSend(const tstring & conferenceName, int memberIndex, bool enable)
{
	if (!isOpenSession()) return false;
	//if (m_account.get() == 0) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("Conference"), conferenceName));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Index"), memberIndex));
	m_cgcClient->doAddParameter(cgcParameter::create2(_T("Enable"), enable));

	m_cgcClient->doSendAppCall(const_CallSign_EnableAudioSend, const_Avs_Api_EnableAudioSend);
	return true;
}

bool CBiwooCgcProxy::enableAudioRecv(const tstring & conferenceName, int memberIndex, bool enable)
{
	if (!isOpenSession()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("Conference"), conferenceName));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Index"), memberIndex));
	m_cgcClient->doAddParameter(cgcParameter::create2(_T("Enable"), enable));

	m_cgcClient->doSendAppCall(const_CallSign_EnableAudioRecv, const_Avs_Api_EnableAudioRecv);
	return true;
}

bool CBiwooCgcProxy::enableVideoSend(const tstring & conferenceName, int memberIndex, bool enable)
{
	if (!isOpenSession()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("Conference"), conferenceName));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Index"), memberIndex));
	m_cgcClient->doAddParameter(cgcParameter::create2(_T("Enable"), enable));

	m_cgcClient->doSendAppCall(const_CallSign_EnableVideoSend, const_Avs_Api_EnableVideoSend);
	return true;
}

bool CBiwooCgcProxy::enableVideoRecv(const tstring & conferenceName, int memberIndex, bool enable)
{
	if (!isOpenSession()) return false;

	m_cgcClient->doAddParameter(cgcParameter::create(_T("Conference"), conferenceName));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Index"), memberIndex));
	m_cgcClient->doAddParameter(cgcParameter::create2(_T("Enable"), enable));

	m_cgcClient->doSendAppCall(const_CallSign_EnableVideoRecv, const_Avs_Api_EnableVideoRecv);
	return true;
}

bool CBiwooCgcProxy::openLocalAV(HWND hWndPreview)
{
	if (m_LDoDSHandler == NULL)
	{
		CAVParameter parameter;
		parameter.preview(hWndPreview);
		parameter.audio(true);
		parameter.audioCoding(AVCoding::CT_G729A);
		parameter.audio(2, 24000);
		parameter.video(true);
		parameter.videoCoding(AVCoding::CT_XVID);
		parameter.video(25, 320, 240);

		m_LDoDSHandler = m_p2pav.OpenLocalAV(parameter);
	}

	return m_LDoDSHandler != NULL;
}

void CBiwooCgcProxy::closeLocalAV(void)
{
	if (m_LDoDSHandler != NULL)
	{
		m_p2pav.CloseLocalAV();
		m_LDoDSHandler = NULL;
	}
}

void CBiwooCgcProxy::closeRemoteAV(void)
{
	if (m_RDoDSHandler != NULL)
	{
		m_p2pav.CloseRemoteAV(m_RDoDSHandler);
		m_RDoDSHandler = NULL;
	}
}

void CBiwooCgcProxy::moveRemoteWindow(void)
{
	if (m_RDoDSHandler != NULL)
	{
		m_RDoDSHandler->MoveWindow();
	}
}

bool CBiwooCgcProxy::videoCall(CUserInfo::pointer calltoUser, HWND hWndPreview)
{
	BOOST_ASSERT (calltoUser.get() != NULL);
	if (!isOpenSession()) return false;
	if (m_account.get() == NULL) return false;

	if (m_RDoDSHandler == NULL)
	{
		CAVParameter parameter;
		parameter.preview(hWndPreview);
		parameter.audio(true);
		parameter.audioCoding(AVCoding::CT_G729A);
		parameter.audio(2, 24000);
		parameter.video(true);
		parameter.reversal(true);
		parameter.videoCoding(AVCoding::CT_XVID);
		parameter.video(0, 320, 240);

		m_RDoDSHandler = m_p2pav.OpenRemoteAV(parameter);
	}

	long mid = time(0)+(++m_currentMID);

	CMessageInfo::pointer messageInfo = CMessageInfo::create(mid, 0,  13, true);
	CSendInfo::pointer sendInfo = CSendInfo::create(calltoUser, messageInfo);
	m_sends.insert(messageInfo->messageid(), sendInfo);

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("SendTo"), calltoUser->getAccount()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("MID"), mid));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Type"), 13));

	m_cgcClient->doSendAppCall(const_CallSign_MsgRequest, const_Api_MsgRequest);
	return true;
}

bool CBiwooCgcProxy::acceptVideoCall(CUserInfo::pointer responsetoUser, HWND hWndPreview, long mid)
{
	BOOST_ASSERT (responsetoUser.get() != NULL);
	if (!isOpenSession()) return false;
	if (m_account.get() == NULL) return false;

	CSendInfo::pointer sendInfo;
	if (!m_recvs.find(mid, sendInfo))
	{
		return false;
	}

	if (m_RDoDSHandler == NULL)
	{
		CAVParameter parameter;
		parameter.preview(hWndPreview);
		parameter.audio(true);
		parameter.audioCoding(AVCoding::CT_G729A);
		parameter.audio(2, 24000);
		parameter.video(true);
		parameter.reversal(true);
		parameter.videoCoding(AVCoding::CT_XVID);
		parameter.video(0, 320, 240);

		m_RDoDSHandler = m_p2pav.OpenRemoteAV(parameter);
	}

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("SendTo"), responsetoUser->getAccount()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("MID"), mid));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Response"), 1));
	m_cgcClient->doSendAppCall(const_CallSign_MsgResponse, const_Api_MsgResponse);

	return m_p2pav.AVP2PConnect(responsetoUser->getAccount());
}

bool CBiwooCgcProxy::releaseVideoCall(CUserInfo::pointer responsetoUser, long mid)
{
	BOOST_ASSERT (responsetoUser.get() != NULL);
	if (!isOpenSession()) return false;
	if (m_account.get() == NULL ) return false;

	CSendInfo::pointer sendInfo;
	if (!m_recvs.find(mid, sendInfo, true))
	{
		return false;
	}

	CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(responsetoUser), m_account->getUserinfo(), sendInfo->msginfo(), CConversationInfo::CT_CANCEL);
	m_unreads.add(conversationInfo);

	m_p2pav.AVP2PDisconnect(responsetoUser->getAccount());

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("SendTo"), responsetoUser->getAccount()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("MID"), mid));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Response"), 3));
	m_cgcClient->doSendAppCall(const_CallSign_MsgResponse, const_Api_MsgResponse);

	closeRemoteAV();
	return true;
}

bool CBiwooCgcProxy::msgSendText(CFromInfo::pointer sendtoUser, const char * sMsg, size_t size, bool bNew)
{
	BOOST_ASSERT (sendtoUser.get() != NULL);

	if (!isOpenSession()) return false;
	if (m_account.get() == NULL || sMsg == NULL) return false;

	char * toSendMessage = const_cast<char*>(sMsg);
	size_t toSendSizeTotal = size;
	short sendCount = size / 6000 + (toSendSizeTotal % 6000 > 0 ? 1 : 0);
	CAccountConversation::pointer accountConversation;
	if (sendtoUser->fromType() == CFromInfo::FromUserInfo)
	{
		if (!m_avs.find(sendtoUser->fromUser()->getAccount(), accountConversation))
		{
			accountConversation = CAccountConversation::create(sendtoUser);
			m_avs.insert(sendtoUser->fromUser()->getAccount(), accountConversation);
		}
	}else if (sendtoUser->fromType() == CFromInfo::FromDialogInfo)
	{
		CDialogInfo::pointer dialogInfo = sendtoUser->fromDialog();
		if (!m_avs2.find(dialogInfo->dialogId(), accountConversation))
		{
			accountConversation = CAccountConversation::create(sendtoUser);
			m_avs2.insert(dialogInfo->dialogId(), accountConversation);
		}

		// ??
		if (dialogInfo->history())
		{
			std::list<CUserInfo::pointer> users;
			CLockMap<tstring, CUserInfo::pointer>::iterator iter;
			for (iter=dialogInfo->m_members.begin(); iter!=dialogInfo->m_members.end(); iter++)
			{
				CUserInfoPointer userInfo = iter->second;
				users.push_back(userInfo);
			}
			users.push_back(m_account->getUserinfo());

			diaMember(dialogInfo->dialogId(), users);
		}
	}

	for (short i=0; i<sendCount; i++)
	{
		size_t sizeSended = 0;
		size_t tosendSize = 0;
		toSendSizeTotal = ((i+1)==sendCount) ? size % 6000 : 6000;
		toSendMessage += i*6000;

		long mid = time(0)+(++m_currentMID);

		struct timeb tbNow;
		ftime(&tbNow);
		tbNow.time -= (tbNow.timezone*60);

		// Save to db.
		CMessageInfo::pointer messageInfo = CMessageInfo::create(mid, toSendSizeTotal, 1, bNew);
		messageInfo->msgtime(tbNow.time);
		messageInfo->setdata(toSendMessage, toSendSizeTotal, 0);
		CConversationInfo::pointer conversationInfo = CConversationInfo::create(sendtoUser, m_account->getUserinfo(), messageInfo);
		accountConversation->m_conversations.add(conversationInfo);
		char sqlBuffer[10*1024];
		memset(sqlBuffer, 0, sizeof(sqlBuffer));
		if (sendtoUser->fromType() == CFromInfo::FromUserInfo)
		{
			sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,sayaccount,message,msgsize,newflag)\
							   VALUES(%d,'%s',1,'%s','%s',%d,%s)",
							   (int)CFromInfo::FromUserInfo, sendtoUser->fromUser()->getAccount().c_str(), 
							   m_account->getUserinfo()->getAccount().c_str(), messageInfo->getdata(),
							   toSendSizeTotal, bNew ? "true" : "false");
		}else if (sendtoUser->fromType() == CFromInfo::FromDialogInfo)
		{
			sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromid,msgtype,fromaccount,sayaccount,message,msgsize,newflag)\
							   VALUES(%d,%ld,1,'%s','%s','%s',%d,%s)",
							   (int)CFromInfo::FromDialogInfo, sendtoUser->fromDialog()->dialogId(),
							   m_account->getUserinfo()->getAccount().c_str(), m_account->getUserinfo()->getAccount().c_str(), 
							   messageInfo->getdata(), toSendSizeTotal, bNew ? "true" : "false");
		}
		bodb_exec(sqlBuffer);

		cgc::cgcAttachment::pointer attach(cgcAttachment::create());
		attach->setName("text");
		attach->setTotal(toSendSizeTotal);
		while (sizeSended < toSendSizeTotal)
		{
			tosendSize = (toSendSizeTotal-sizeSended) > MAX_PACKET_SIZE ? MAX_PACKET_SIZE : (toSendSizeTotal-sizeSended);
			attach->setAttach((const unsigned char*)toSendMessage+sizeSended, tosendSize);
			attach->setIndex(sizeSended);

			m_cgcClient->doBeginCallLock();
			m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
			m_cgcClient->doAddParameter(cgcParameter::create(_T("MID"), mid));
			switch (sendtoUser->fromType())
			{
			case CFromInfo::FromUserInfo:
				m_cgcClient->doAddParameter(cgcParameter::create(_T("SendTo"), sendtoUser->fromUser()->getAccount()));
				break;
			case CFromInfo::FromDialogInfo:
				m_cgcClient->doAddParameter(cgcParameter::create(_T("DID"), sendtoUser->fromDialog()->dialogId()));
				break;
			default:
				return false;
				break;
			}
			if (attach->getIndex() == 0)
			{
				m_cgcClient->doAddParameter(cgcParameter::create(_T("Type"), 1));
				m_cgcClient->doAddParameter(cgcParameter::create2(_T("New"), bNew));
			}
			m_cgcClient->doSendAppCall(const_CallSign_MsgSend, const_Api_MsgSend, attach);
			sizeSended += tosendSize;

#ifdef WIN32
			Sleep(5);
#else
			usleep(5000);
#endif
		}
		bNew = false;
	}
	return true;
}

bool CBiwooCgcProxy::msgSendImage(CFromInfo::pointer sendtoUser, int width, int height, int imagetype, const char * sMsg, size_t size, bool bNew)
{
	BOOST_ASSERT (sendtoUser.get() != NULL);

	if (!isOpenSession()) return false;
	if (m_account.get() == NULL || sMsg == NULL) return false;

	size_t sizeSended = 0;
	size_t tosendSize = 0;

	char * toSendMessage = const_cast<char*>(sMsg);

	int scalewidth = width % 2;
	int scaleheight = height % 2;
	if (scalewidth > 0 || scaleheight > 0)
	{
		SwsContext * m_swsContextDecode = sws_getContext(
			width, height, PIX_FMT_BGR24,
			width-scalewidth, height-scaleheight, PIX_FMT_BGR24,
			SWS_BICUBIC,
			NULL, NULL, NULL);

		if (m_swsContextDecode == NULL)
			return false;

		unsigned char * srcdata[4];
		srcdata[0] = (unsigned char*)const_cast<char*>(sMsg);
		int srclinesize[4];
		memset(srclinesize, 0, sizeof(srclinesize));
		srclinesize[0] = width * 3;

		unsigned char * destdata[4];
		destdata[0] = new unsigned char[size];
		int destlinesize[4];
		memset(destlinesize, 0, sizeof(destlinesize));
		destlinesize[0] = (width-scalewidth) * 3;

		int ret = sws_scale(m_swsContextDecode, srcdata, srclinesize, 0, height, destdata, destlinesize);

		sws_freeContext(m_swsContextDecode);

		width -= scalewidth;
		height -= scaleheight;
		toSendMessage = (char*)destdata[0];
		size = width * height * 3;
	}

	CXvidProxy m_xvid;
	XInfo xinfo(XVID_CSP_BGR, 24, width, height);
	m_xvid.setenc_info(xinfo);

	unsigned char * encode_buffer;
	int encode_size = 0;

	if (!m_xvid.encode((const unsigned char*)toSendMessage, &encode_buffer, encode_size))
	{
		if (toSendMessage != sMsg)
		{
			delete[] toSendMessage;
		}
		return false;
	}
	if (toSendMessage != sMsg)
	{
		delete[] toSendMessage;
	}

	// ?
	long mid = encode_size+(++m_currentMID);

	// Save the image.
	char filename[50];
	sprintf(filename, "%I64d%08d", time(0), mid);
	char filepath[256];
	sprintf(filepath, "%s/users/%s/File/%s", this->m_sModulePath.c_str(), m_account->getUserinfo()->getAccount().c_str(), filename);
	FILE * hfile = fopen(filepath, "wb");
	if (hfile != NULL)
	{
		fwrite(encode_buffer, 1, encode_size, hfile);
		fclose(hfile);
	}

	CAccountConversation::pointer accountConversation;
	if (sendtoUser->fromType() == CFromInfo::FromUserInfo)
	{
		if (!m_avs.find(sendtoUser->fromUser()->getAccount(), accountConversation))
		{
			accountConversation = CAccountConversation::create(sendtoUser);
			m_avs.insert(sendtoUser->fromUser()->getAccount(), accountConversation);
		}
	}else if (sendtoUser->fromType() == CFromInfo::FromDialogInfo)
	{
		CDialogInfo::pointer dialogInfo = sendtoUser->fromDialog();
		if (!m_avs2.find(dialogInfo->dialogId(), accountConversation))
		{
			accountConversation = CAccountConversation::create(sendtoUser);
			m_avs2.insert(dialogInfo->dialogId(), accountConversation);
		}

		// ??
		if (dialogInfo->history())
		{
			std::list<CUserInfo::pointer> users;
			CLockMap<tstring, CUserInfo::pointer>::iterator iter;
			for (iter=dialogInfo->m_members.begin(); iter!=dialogInfo->m_members.end(); iter++)
			{
				CUserInfoPointer userInfo = iter->second;
				users.push_back(userInfo);
			}
			users.push_back(m_account->getUserinfo());

			diaMember(dialogInfo->dialogId(), users);
		}
	}

	struct timeb tbNow;
	ftime(&tbNow);
	tbNow.time -= (tbNow.timezone*60);

	CMessageInfo::pointer messageInfo = CMessageInfo::create(mid, size, 3, bNew);
	messageInfo->msgtime(tbNow.time);
	messageInfo->setdata((const char*)toSendMessage, size, 0);
	CConversationInfo::pointer conversationInfo = CConversationInfo::create(sendtoUser, m_account->getUserinfo(), messageInfo);
	accountConversation->m_conversations.add(conversationInfo);

	// Save to db.
	char sqlBuffer[10*1024];
	memset(sqlBuffer, 0, sizeof(sqlBuffer));
	if (sendtoUser->fromType() == CFromInfo::FromUserInfo)
	{
		sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,sayaccount,message,msgsize,width,height,newflag)\
						   VALUES(%d,'%s',3,'%s','%s',%d,%d,%d,%s)",
						   (int)CFromInfo::FromUserInfo, sendtoUser->fromUser()->getAccount().c_str(),
						   m_account->getUserinfo()->getAccount().c_str(), filename,
						   encode_size, width, height, bNew ? "true" : "false");
	}else if (sendtoUser->fromType() == CFromInfo::FromDialogInfo)
	{
		sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromid,msgtype,sayaccount,message,msgsize,width,height,newflag)\
						   VALUES(%d,%ld,3,'%s','%s',%d,%d,%d,%s)",
						   (int)CFromInfo::FromDialogInfo, sendtoUser->fromDialog()->dialogId(),
						   m_account->getUserinfo()->getAccount().c_str(), filename,
						   encode_size, width, height, bNew ? "true" : "false");
	}
	bodb_exec(sqlBuffer);

	cgc::cgcAttachment::pointer attach(cgcAttachment::create());
	attach->setName("image");
	attach->setTotal(encode_size);
	while ((int)sizeSended < encode_size)
	{
		tosendSize = (encode_size-sizeSended) > MAX_PACKET_SIZE ? MAX_PACKET_SIZE : (encode_size-sizeSended);
		attach->setAttach((const unsigned char*)encode_buffer+sizeSended, tosendSize);
		attach->setIndex(sizeSended);
		m_cgcClient->doBeginCallLock();
		m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
		m_cgcClient->doAddParameter(cgcParameter::create(_T("MID"), mid));
		switch (sendtoUser->fromType())
		{
		case CFromInfo::FromUserInfo:
			m_cgcClient->doAddParameter(cgcParameter::create(_T("SendTo"), sendtoUser->fromUser()->getAccount()));
			break;
		case CFromInfo::FromDialogInfo:
			m_cgcClient->doAddParameter(cgcParameter::create(_T("DID"), sendtoUser->fromDialog()->dialogId()));
			break;
		default:
			return false;
			break;
		}
		if (sizeSended == 0)
		{
			m_cgcClient->doAddParameter(cgcParameter::create(_T("Type"), 3));
			m_cgcClient->doAddParameter(cgcParameter::create(_T("Width"), width));
			m_cgcClient->doAddParameter(cgcParameter::create(_T("Height"), height));
			m_cgcClient->doAddParameter(cgcParameter::create2(_T("New"), bNew));
		}
		m_cgcClient->doSendAppCall(const_CallSign_MsgSend, const_Api_MsgSend, attach);
		sizeSended += tosendSize;

#ifdef WIN32
		Sleep(5);
#else
		usleep(5000);
#endif
	}
	delete[] encode_buffer;

	return true;
}

bool CBiwooCgcProxy::msgSendFile(CUserInfo::pointer sendtoUser, const char * filepath, const char * filename)
{
	BOOST_ASSERT (sendtoUser.get() != NULL);

	if (!isOpenSession()) return false;
	if (m_account.get() == NULL || filepath == NULL || filename == NULL) return false;

	struct timeb tbNow;
	ftime(&tbNow);
	tbNow.time -= (tbNow.timezone*60);

	CMessageInfo::pointer messageInfo = CMessageInfo::create(time(0)+(++m_currentMID), strlen(filename),  11, true);
	messageInfo->msgtime(tbNow.time);
	messageInfo->setdata(filename, messageInfo->total(), 0);

	CSendInfo::pointer sendInfo = CSendInfo::create(sendtoUser, messageInfo);
	sendInfo->filename(filename);

	//setlocale(LC_ALL,"Chinese-simplified");
	//std::locale::global(std::locale(""));
	tfstream & filestream = sendInfo->fs();
	filestream.open(filepath, std::ios::in|std::ios::binary);
	if (!filestream.is_open())
	{
		return false;
	}

	filestream.seekg(0, std::ios::end);
	unsigned int nFilesize = filestream.tellg();
	filestream.seekg(0, std::ios::beg);

	messageInfo->filesize(nFilesize);
	m_sends.insert(messageInfo->messageid(), sendInfo);

	// Save to db.
	char sqlBuffer[10*1024];
	memset(sqlBuffer, 0, sizeof(sqlBuffer));
	sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,sayaccount,message,msgsize,newflag)\
					   VALUES(%d,'%s',11,'%s','%s',%d,true)",
					   (int)CFromInfo::FromUserInfo, sendtoUser->getAccount().c_str(),
					   m_account->getUserinfo()->getAccount().c_str(), filename,
					   nFilesize);
	bodb_exec(sqlBuffer);

	// p2p first
	long p2pmid = m_p2pav.SendFile(sendtoUser->getAccount(), filepath, filename);
	if (p2pmid > 0)
	{
		m_sends.remove(messageInfo->messageid());
		messageInfo->messageid(p2pmid);
		m_sends.insert(messageInfo->messageid(), sendInfo);

		CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(sendInfo->fromUser()), m_account->getUserinfo(), sendInfo->msginfo());
		m_unreads.add(conversationInfo);

		return true;
	}

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("SendTo"), sendtoUser->getAccount()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("MID"), messageInfo->messageid()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Type"), 11));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Name"), filename));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Size"), nFilesize));

	m_cgcClient->doSendAppCall(const_CallSign_MsgRequest, const_Api_MsgRequest);
	return true;
}

bool CBiwooCgcProxy::msgAcceptFile(CUserInfo::pointer responsetoUser, long mid, const char * savetofilepath, const char * filename)
{
	BOOST_ASSERT (responsetoUser.get() != NULL);
	if (!isOpenSession()) return false;
	if (m_account.get() == NULL || savetofilepath == NULL || filename == NULL) return false;

	m_rejects.remove(mid);

	CSendInfo::pointer sendInfo;
	if (!m_recvs.find(mid, sendInfo))
	{
		return false;
	}
	sendInfo->accept(true);

	sendInfo->filepath(savetofilepath);
	sendInfo->filename(filename);

	// Save to db.
	char sqlBuffer[10*1024];
	memset(sqlBuffer, 0, sizeof(sqlBuffer));
	sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,subtype,sayaccount,message,msgsize,newflag)\
					   VALUES(%d,'%s',11,%d,'%s','%s',%d,true)",
					   (int)CFromInfo::FromUserInfo, responsetoUser->getAccount().c_str(), (int)CConversationInfo::CT_ACCEPT,
					   m_account->getUserinfo()->getAccount().c_str(), filename,
					   sendInfo->msginfo()->filesize());
	bodb_exec(sqlBuffer);

	// p2p first
	if (m_p2pav.AcceptFile(mid, savetofilepath, filename))
	{
		return true;
	}

	tfstream & filestream = sendInfo->fs();
	filestream.open(savetofilepath, std::ios::out|std::ios::binary);
	if (!filestream.is_open())
	{
		return false;
	}
	char * buffer = new char[sendInfo->msginfo()->filesize()];
	memset(buffer, 0, sendInfo->msginfo()->filesize());
	filestream.write(buffer, sendInfo->msginfo()->filesize());
	filestream.flush();
	delete[] buffer;


	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("SendTo"), responsetoUser->getAccount()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("MID"), sendInfo->msginfo()->messageid()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Response"), 1));
	m_cgcClient->doSendAppCall(const_CallSign_MsgResponse, const_Api_MsgResponse);
	return true;
}

bool CBiwooCgcProxy::msgRejectFile(CUserInfo::pointer responsetoUser, long mid)
{
	BOOST_ASSERT (responsetoUser.get() != NULL);
	if (!isOpenSession()) return false;
	if (m_account.get() == NULL ) return false;

	m_rejects.insert(mid, true);

	CSendInfo::pointer sendInfo;
	if (!m_recvs.find(mid, sendInfo, true))
	{
		return false;
	}
	sendInfo->accept(false);

	std::string filename = sendInfo->filename().empty() ? sendInfo->msginfo()->tostring() : sendInfo->filename();

	// Save to db.
	char sqlBuffer[10*1024];
	memset(sqlBuffer, 0, sizeof(sqlBuffer));
	sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,subtype,sayaccount,message,msgsize,newflag)\
					   VALUES(%d,'%s',11,%d,'%s','%s',%d,true)",
					   (int)CFromInfo::FromUserInfo, responsetoUser->getAccount().c_str(), (int)CConversationInfo::CT_REJECT,
					   m_account->getUserinfo()->getAccount().c_str(), filename.c_str(),
					   sendInfo->msginfo()->filesize());
	bodb_exec(sqlBuffer);

	CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(responsetoUser), m_account->getUserinfo(), sendInfo->msginfo(), CConversationInfo::CT_REJECT);
	m_unreads.add(conversationInfo);

	if (m_p2pav.RejectFile(mid))
	{
		return true;
	}

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("SendTo"), responsetoUser->getAccount()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("MID"), sendInfo->msginfo()->messageid()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Response"), 2));
	m_cgcClient->doSendAppCall(const_CallSign_MsgResponse, const_Api_MsgResponse);
	return true;
}

bool CBiwooCgcProxy::msgCancelFile(CUserInfo::pointer responsetoUser, long mid)
{
	BOOST_ASSERT (responsetoUser.get() != NULL);
	if (!isOpenSession()) return false;
	if (m_account.get() == NULL ) return false;

	m_rejects.insert(mid, true);

	CSendInfo::pointer sendInfo;
	if (!m_sends.find(mid, sendInfo, true))
	{
		return false;
	}
	sendInfo->cancel();

	// Save to db.
	char sqlBuffer[10*1024];
	memset(sqlBuffer, 0, sizeof(sqlBuffer));
	sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,subtype,sayaccount,message,msgsize,newflag)\
					   VALUES(%d,'%s',11,%d,'%s','%s',%d,true)",
					   (int)CFromInfo::FromUserInfo, responsetoUser->getAccount().c_str(), (int)CConversationInfo::CT_CANCEL,
					   m_account->getUserinfo()->getAccount().c_str(), sendInfo->msginfo()->tostring().c_str(),
					   sendInfo->msginfo()->filesize());
	bodb_exec(sqlBuffer);

	if (m_p2pav.CancelFile(mid))
	{
		CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(responsetoUser), m_account->getUserinfo(), sendInfo->msginfo(), CConversationInfo::CT_CANCEL);
		m_unreads.add(conversationInfo);
		return true;
	}

	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("SendTo"), responsetoUser->getAccount()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("MID"), sendInfo->msginfo()->messageid()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("Response"), 3));
	m_cgcClient->doSendAppCall(const_CallSign_MsgResponse, const_Api_MsgResponse);
	return true;
}

bool CBiwooCgcProxy::diaInvite(long dialogId, const std::list<CUserInfo::pointer> & users)
{
	if (!isOpenSession()) return false;
	if (m_account.get() == NULL ) return false;
	if (users.empty()) return false;

	m_cgcClient->doBeginCallLock();
	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("DID"), dialogId));

	int index = 0;
	std::list<CUserInfo::pointer>::const_iterator iter;
	for (iter=users.begin(); iter!=users.end(); iter++)
	{
		CUserInfo::pointer inviteUserInfo = *iter;

		char buffer[10];
		sprintf(buffer, "UA%d", index++);
		m_cgcClient->doAddParameter(cgcParameter::create(buffer, inviteUserInfo->getAccount()));
	}

	m_cgcClient->doSendAppCall(const_CallSign_DiaInvite, const_Api_DiaInvite);
	return true;
}

bool CBiwooCgcProxy::diaMember(long dialogId, const std::list<CUserInfo::pointer> & users)
{
	if (!isOpenSession()) return false;
	if (m_account.get() == NULL ) return false;
	if (users.empty()) return false;

	m_cgcClient->doBeginCallLock();
	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("DID"), dialogId));

	int index = 0;
	std::list<CUserInfo::pointer>::const_iterator iter;
	for (iter=users.begin(); iter!=users.end(); iter++)
	{
		CUserInfo::pointer inviteUserInfo = *iter;

		char buffer[10];
		sprintf(buffer, "UA%d", index++);
		m_cgcClient->doAddParameter(cgcParameter::create(buffer, inviteUserInfo->getAccount()));
	}

	m_cgcClient->doSendAppCall(const_CallSign_DiaMember, const_Api_DiaMember);
	return true;
}

bool CBiwooCgcProxy::diaQuit(long dialogId)
{
	if (!isOpenSession()) return false;
	if (m_account.get() == NULL ) return false;

	//if (!m_dialogs.remove(dialogId))
	//{
	//	// ?
	//	return true;
	//}

	m_cgcClient->doBeginCallLock();
	m_cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
	m_cgcClient->doAddParameter(cgcParameter::create(_T("DID"), dialogId));
	m_cgcClient->doSendAppCall(const_CallSign_DiaQuit, const_Api_DiaQuit);
	return true;
}

CUserInfo::pointer CBiwooCgcProxy::getUserInfo(const std::string & userAccount) const
{
	CUserInfo::pointer result;

	if (m_account.get() == 0)
	{
		return result;
	}

	CFriendInfo::pointer friendInfo;
	if (m_account->m_allfriends.find(userAccount, friendInfo))
	{
		result = friendInfo->userinfo();
	}else
	{
		/////////////////////////////////
		// Response
		CLockMap<unsigned int, CCompanyInfoPointer>::const_iterator iterCompany;
		boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(m_account->getUserinfo()->m_companys.mutex()));
		for (iterCompany=m_account->getUserinfo()->m_companys.begin(); iterCompany!=m_account->getUserinfo()->m_companys.end(); iterCompany++)
		{
			result = iterCompany->second->getUserInfo(userAccount);
			if (result.get() != 0)
				break;
		}
	}
	return result;
}

CUserInfo::pointer CBiwooCgcProxy::getUserInfo(const std::string & userAccount, CCoGroupInfo::pointer & outCogroupInfo) const
{
	CUserInfo::pointer resultNull;

	if (m_account.get() == 0)
	{
		return resultNull;
	}

	CLockMap<unsigned int, CCompanyInfoPointer>::const_iterator iterCompany;
	boost::mutex::scoped_lock lockCompany(const_cast<boost::mutex&>(m_account->getUserinfo()->m_companys.mutex()));
	for (iterCompany=m_account->getUserinfo()->m_companys.begin(); iterCompany!=m_account->getUserinfo()->m_companys.end(); iterCompany++)
	{
		CCompanyInfoPointer companyInfo = iterCompany->second;

		CLockMap<unsigned int, CCoGroupInfo::pointer>::const_iterator iterCoGroup;
		//boost::mutex::scoped_lock lockCoGroupInfo(const_cast<boost::mutex&>(companyInfo->m_allgroups.mutex()));
		for (iterCoGroup=companyInfo->m_allgroups.begin(); iterCoGroup!=companyInfo->m_allgroups.end(); iterCoGroup++)
		{
			CCoGroupInfo::pointer cogroupInfo = iterCoGroup->second;
			if (cogroupInfo->type() == CCoGroupInfo::GT_Normal)
			{
				CUserInfo::pointer fromUserInfo;
				if (cogroupInfo->m_userinfos.find(userAccount, fromUserInfo))
				{
					outCogroupInfo = cogroupInfo;
					return fromUserInfo;
				}
			}
		}
	}

	return resultNull;
}

CCoGroupInfo::pointer CBiwooCgcProxy::getUserInfo(CUserInfo::pointer userAccount) const
{
	BOOST_ASSERT (userAccount.get() != NULL);

	CCoGroupInfoPointer resultNull;
	CLockMap<unsigned int, CCoGroupInfoPointer>::const_iterator iterCoGroup;
	boost::mutex::scoped_lock lockCoGroup(const_cast<boost::mutex&>(userAccount->m_cogroups.mutex()));
	for (iterCoGroup=userAccount->m_cogroups.begin(); iterCoGroup!=userAccount->m_cogroups.end(); iterCoGroup++)
	{
		CCoGroupInfoPointer cogroupInfo = iterCoGroup->second;
		if (cogroupInfo->type() == CCoGroupInfo::GT_Normal)
		{
			return cogroupInfo;
		}
	}
	return resultNull;
}

void CBiwooCgcProxy::doStaProcSend(CBiwooCgcProxy * pProxy, CSendInfo::pointer sendInfo)
{
	BOOST_ASSERT (pProxy != NULL);
	pProxy->doProcSend(sendInfo);
}

void CBiwooCgcProxy::doProcSend(CSendInfo::pointer sendInfo)
{
	BOOST_ASSERT (sendInfo.get() != NULL);
	BOOST_ASSERT (sendInfo->fs().is_open());

	CMessageInfo::pointer messageInfo  = sendInfo->msginfo();
	CUserInfo::pointer sendtoUser = sendInfo->fromUser();
	while (!sendInfo->responsed() || sendInfo->accepted())
	{
		if (sendInfo->canceled())
		{
			CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(sendtoUser), m_account->getUserinfo(), messageInfo, CConversationInfo::CT_CANCEL);
			m_unreads.add(conversationInfo);
			break;
		}

		if (!sendInfo->accepted())
		{
#ifdef WIN32
			Sleep(500);
#else
			usleep(500000);
#endif
			continue;
		}

		// Accepted
		std::string filename = messageInfo->tostring();
		long mid = messageInfo->messageid();
		bool bNew = messageInfo->newflag();

		tfstream & filestream = sendInfo->fs();
		filestream.seekg(0, std::ios::beg);

		size_t toSendSizeTotal = messageInfo->filesize();
		size_t sizeSended = 0;
		char buffer[MAX_PACKET_SIZE+1];

		cgc::cgcAttachment::pointer attach(cgcAttachment::create());
		attach->setName("file");
		attach->setTotal(toSendSizeTotal);
		while (sizeSended < toSendSizeTotal)
		{
			size_t tosendSize = (toSendSizeTotal-sizeSended) > MAX_PACKET_SIZE ? MAX_PACKET_SIZE : (toSendSizeTotal-sizeSended);

			memset(buffer, 0, MAX_PACKET_SIZE+1);
			filestream.read(buffer, tosendSize);
			attach->setAttach((const unsigned char *)buffer, tosendSize);
			attach->setIndex(sizeSended);

			//DoSotpClientHandler * cgcClient = m_cgcClient;
			DoSotpClientHandler * cgcClient = m_fileClient;

			cgcClient->doBeginCallLock();
			cgcClient->doAddParameter(cgcParameter::create(_T("AccountId"), m_account->getAccountId()));
			cgcClient->doAddParameter(cgcParameter::create(_T("MID"), mid));
			cgcClient->doAddParameter(cgcParameter::create(_T("SendTo"), sendtoUser->getAccount()));
			if (attach->getIndex() == 0)
			{
				cgcClient->doAddParameter(cgcParameter::create(_T("Type"), 11));
				cgcClient->doAddParameter(cgcParameter::create2(_T("New"), bNew));
			}
			cgcClient->doSendAppCall(const_CallSign_MsgSend, const_Api_MsgSend, attach);
			sizeSended += tosendSize;

			size_t indexCount = toSendSizeTotal / MAX_PACKET_SIZE + 1;

			if (sizeSended == toSendSizeTotal)
			{
				// Save to db.
				char sqlBuffer[10*1024];
				memset(sqlBuffer, 0, sizeof(sqlBuffer));
				sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,subtype,sayaccount,message,msgsize,newflag)\
								   VALUES(%d,'%s',11,%d,'%s','%s',%d,true)",
								   (int)CFromInfo::FromUserInfo, sendtoUser->getAccount().c_str(), (int)CConversationInfo::CT_SUCCEED,
								   m_account->getUserinfo()->getAccount().c_str(), sendInfo->filename().c_str(),
								   sendInfo->msginfo()->filesize());
				bodb_exec(sqlBuffer);

				CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(sendtoUser), m_account->getUserinfo(), messageInfo);
				conversationInfo->readState(false);
				conversationInfo->percent(100.0);
				m_unreads.add(conversationInfo);
			}else if (sizeSended == tosendSize)
			{
				CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(sendtoUser), m_account->getUserinfo(), messageInfo);
				conversationInfo->readState(false);
				conversationInfo->percent(0.00);
				m_unreads.add(conversationInfo);
			}else
			{
				size_t eventCount = indexCount / 80;
				eventCount = eventCount < 20 ? 20 : eventCount;
				//static size_t receiveCount = 0;

				//if (++receiveCount == eventCount)
				if (messageInfo->icounter() == eventCount)
				{
					messageInfo->resetcounter();
					//receiveCount = 0;

					CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(sendtoUser), m_account->getUserinfo(), messageInfo);
					conversationInfo->readState(false);
					conversationInfo->percent(sizeSended * 100.0 / toSendSizeTotal);
					m_unreads.add(conversationInfo);
				}
			}

			if (sendInfo->canceled()) 
			{
				CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(sendtoUser), m_account->getUserinfo(), messageInfo, CConversationInfo::CT_CANCEL);
				m_unreads.add(conversationInfo);
				break;
			}
			if (!sendInfo->accepted())
			{
				break;
			}

#ifdef WIN32
			Sleep(9);
#else
			usleep(9000);
#endif
		}

		// OK
		break;
	}
}

void CBiwooCgcProxy::onFileRequest(const tstring & fromAccount, long fid, const tstring & sFilename, size_t nFilesize)
{
	CUserInfo::pointer fromUserInfo = getUserInfo(fromAccount);
	if (fromUserInfo.get() == 0)
	{
		return;
	}

	long mid = fid;
	// Save to db.
	char sqlBuffer[10*1024];
	memset(sqlBuffer, 0, sizeof(sqlBuffer));
	sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,sayaccount,message,msgsize,newflag)\
					   VALUES(%d,'%s',11,'%s','%s',%d,true)",
					   (int)CFromInfo::FromUserInfo, fromAccount.c_str(), 
					   fromAccount.c_str(), sFilename.c_str(),
					   nFilesize);
	bodb_exec(sqlBuffer);

	CMessageInfo::pointer messageInfo = CMessageInfo::create(mid, sFilename.size(),  11, true);
	messageInfo->setdata(sFilename.c_str(), sFilename.size(), 0);
	messageInfo->filesize(nFilesize);

	struct timeb tbNow;
	ftime(&tbNow);
	tbNow.time -= (tbNow.timezone*60);
	messageInfo->msgtime(tbNow.time);

	m_recvs.insert(messageInfo->messageid(), CSendInfo::create(fromUserInfo, messageInfo));

	CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(fromUserInfo), fromUserInfo, messageInfo);
	conversationInfo->readState(false);
	m_unreads.add(conversationInfo);

}
	
void CBiwooCgcProxy::onFileRejected(const tstring & fromAccount, long fid)
{
	CUserInfo::pointer fromUserInfo = getUserInfo(fromAccount);
	if (fromUserInfo.get() == 0)
	{
		return;
	}

	long mid = fid;
	CSendInfo::pointer sendInfo;
	if (!m_sends.find(mid, sendInfo, true))
	{
		return;
	}

	CConversationInfo::ConversationType conversationType = CConversationInfo::CT_REJECT;

	// Save to db.
	char sqlBuffer[10*1024];
	memset(sqlBuffer, 0, sizeof(sqlBuffer));
	sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,subtype,sayaccount,message,msgsize,newflag)\
					   VALUES(%d,'%s',11,%d,'%s','%s',%d,true)",
					   (int)CFromInfo::FromUserInfo, fromAccount.c_str(), conversationType,
					   fromAccount.c_str(), sendInfo->msginfo()->tostring().c_str(),
					   sendInfo->msginfo()->filesize());
	bodb_exec(sqlBuffer);
	CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(sendInfo->fromUser()), sendInfo->fromUser(), sendInfo->msginfo(), conversationType);
	m_unreads.add(conversationInfo);
}

void CBiwooCgcProxy::onFileCanceled(const tstring & fromAccount, long fid)
{
	CUserInfo::pointer fromUserInfo = getUserInfo(fromAccount);
	if (fromUserInfo.get() == 0)
	{
		return;
	}

	long mid = fid;
	CSendInfo::pointer sendInfo;
	if (!m_recvs.find(mid, sendInfo, true))
	{
		return;
	}


	std::string filename = sendInfo->filename().empty() ? sendInfo->msginfo()->tostring() : sendInfo->filename();
	// Save to db.
	char sqlBuffer[10*1024];
	memset(sqlBuffer, 0, sizeof(sqlBuffer));
	sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,subtype,sayaccount,message,msgsize,newflag)\
					   VALUES(%d,'%s',11,%d,'%s','%s',%d,true)",
					   (int)CFromInfo::FromUserInfo, fromAccount.c_str(), (int)CConversationInfo::CT_CANCEL,
					   fromAccount.c_str(), filename.c_str(),
					   sendInfo->msginfo()->filesize());
	bodb_exec(sqlBuffer);

	CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(sendInfo->fromUser()), sendInfo->fromUser(), sendInfo->msginfo(), CConversationInfo::CT_CANCEL);
	m_unreads.add(conversationInfo);
}

void CBiwooCgcProxy::onSendFileData(const tstring & fromAccount, long fid, float percent)
{
	CUserInfo::pointer fromUserInfo = getUserInfo(fromAccount);
	if (fromUserInfo.get() == 0)
	{
		return;
	}

	long mid = fid;
	CSendInfo::pointer sendInfo;
	if (!m_sends.find(mid, sendInfo, percent == 100.0 ? true : false))
	{
		return;
	}

	CMessageInfo::pointer messageInfo = sendInfo->msginfo();

	if (percent == 0.0)
	{
		CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(fromUserInfo), m_account->getUserinfo(), messageInfo);
		conversationInfo->readState(false);
		conversationInfo->percent(percent);
		m_unreads.add(conversationInfo);

	}else if (percent == 100.0)
	{
		// Save to db.
		char sqlBuffer[1024];
		memset(sqlBuffer, 0, sizeof(sqlBuffer));
		sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,subtype,sayaccount,message,msgsize,newflag)\
						   VALUES(%d,'%s',11,%d,'%s','%s',%d,true)",
						   (int)CFromInfo::FromUserInfo, fromAccount.c_str(), (int)CConversationInfo::CT_SUCCEED,
						   m_account->getUserinfo()->getAccount().c_str(), sendInfo->filename().c_str(),
						   sendInfo->msginfo()->filesize());
		bodb_exec(sqlBuffer);

		CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(fromUserInfo), m_account->getUserinfo(), messageInfo);
		conversationInfo->readState(false);
		conversationInfo->percent(percent);
		m_unreads.add(conversationInfo);
	}else
	{
		CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(fromUserInfo), m_account->getUserinfo(), messageInfo);
		conversationInfo->readState(false);
		conversationInfo->percent(percent);
		m_unreads.add(conversationInfo);
	}
}

void CBiwooCgcProxy::onReceiveFileData(const tstring & fromAccount, long fid, float percent)
{
	CUserInfo::pointer fromUserInfo = getUserInfo(fromAccount);
	if (fromUserInfo.get() == 0)
	{
		return;
	}

	long mid = fid;
	CSendInfo::pointer sendInfo;
	if (!m_recvs.find(mid, sendInfo, percent == 100.0 ? true : false))
	{
		return;
	}

	CMessageInfo::pointer messageInfo = sendInfo->msginfo();

	if (percent == 0.0)
	{
		CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(fromUserInfo), fromUserInfo, messageInfo);
		conversationInfo->readState(false);
		conversationInfo->percent(percent);
		m_unreads.add(conversationInfo);

	}else if (percent == 100.0)
	{
		// Save to db.
		char sqlBuffer[10*1024];
		memset(sqlBuffer, 0, sizeof(sqlBuffer));
		sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,subtype,sayaccount,message,msgsize,newflag)\
						   VALUES(%d,'%s',11,%d,'%s','%s',%d,true)",
						   (int)CFromInfo::FromUserInfo, fromAccount.c_str(), (int)CConversationInfo::CT_SUCCEED,
						   fromAccount.c_str(), sendInfo->filepath().c_str(),
						   sendInfo->msginfo()->filesize());
		bodb_exec(sqlBuffer);

		struct timeb tbNow;
		ftime(&tbNow);
		tbNow.time -= (tbNow.timezone*60);
		messageInfo->msgtime(tbNow.time);
		messageInfo->resetdata(sendInfo->filepath().c_str(), sendInfo->filepath().size());

		CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(fromUserInfo), fromUserInfo, messageInfo, CConversationInfo::CT_SUCCEED);
		conversationInfo->readState(false);
		conversationInfo->percent(percent);
		m_unreads.add(conversationInfo);
	}else
	{
		CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(fromUserInfo), fromUserInfo, messageInfo);
		conversationInfo->readState(false);
		conversationInfo->percent(percent);
		m_unreads.add(conversationInfo);
	}
}

void CBiwooCgcProxy::OnCgcResponse(const cgcParser & response)
{
	if (response.isResulted() && response.isOpenType())
	{
		return;
	}
	if (m_handler == NULL || m_cgcClient == NULL) return;

	long resultValue = response.getResultValue();
	switch (response.getSign())
	{
	case 602:
		{
			if (m_account.get() == NULL) break;

			long dialogId = response.getRecvParameterValue(_T("DID"), 0);
			if (dialogId == 0)
			{
				break;
			}
			const tstring & fromAccount = response.getRecvParameterValue(_T("FromAccount"));
			CUserInfo::pointer fromUserInfo = getUserInfo(fromAccount);
			if (fromUserInfo.get() == NULL)
			{
				// error
				break;
			}

			CDialogInfo::pointer dialogInfo = this->getDialogInfo(dialogId);
			if (dialogInfo.get() == NULL)
			{
				// error
				break;
			}
			if (dialogInfo->m_members.remove(fromAccount))
			{
				char sqlBuffer[1024];
				memset(sqlBuffer, 0, sizeof(sqlBuffer));
				sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromid,msgtype,subtype,fromaccount,sayaccount)\
								   VALUES(%d,%ld,602,%d,'%s','%s')",
								   (int)CFromInfo::FromDialogInfo, dialogId, (int)CConversationInfo::CT_QUIT,
								   fromAccount.c_str(), fromAccount.c_str());
				bodb_exec(sqlBuffer);

				CMessageInfo::pointer messageInfo = CMessageInfo::create(0, 0, 602, true);
				CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(dialogInfo), fromUserInfo, messageInfo, CConversationInfo::CT_QUIT);
				m_unreads.add(conversationInfo);
			}

		}break;
	case 601:
		{
			if (m_account.get() == NULL) break;

			long dialogId = response.getRecvParameterValue(_T("DID"), 0);
			if (dialogId == 0)
			{
				break;
			}

			struct timeb tbNow;
			ftime(&tbNow);
			tbNow.time -= (tbNow.timezone*60);
			time_t msgTime = response.getRecvParameterValue(_T("Time"), tbNow.time);
			const tstring & fromAccount = response.getRecvParameterValue(_T("FromAccount"));

			CUserInfo::pointer fromUserInfo;
			if (fromAccount == m_account->getUserinfo()->getAccount())
				fromUserInfo = m_account->getUserinfo();
			else
			{
				fromUserInfo = getUserInfo(fromAccount);
				if (fromUserInfo.get() == NULL)
				{
					// error
					break;
				}
			}

			CDialogInfo::pointer dialogInfo;
			m_dialogs.find(dialogId, dialogInfo);

			int index = 0;
			while (true)
			{
				char bufferUa[10];
				sprintf(bufferUa, "UA%d", index++);

				const tstring & memberUserAccount = response.getRecvParameterValue((tstring)bufferUa);
				if (memberUserAccount.empty())
				{
					break;
				}

				bool ownerMemberAccount = memberUserAccount == m_account->getUserinfo()->getAccount();
				CUserInfo::pointer memberUserInfo;
				if (ownerMemberAccount)
				{
					memberUserInfo = m_account->getUserinfo();
				}else
				{
					memberUserInfo = getUserInfo(memberUserAccount);
					if (memberUserInfo.get() == NULL)
					{
						break;
					}
				}
				BOOST_ASSERT (memberUserInfo.get() != NULL);

				// ??
				CConversationInfo::pointer conversationInfo;
				CMessageInfo::pointer messageInfo = CMessageInfo::create(0, 0, 601, true);
				messageInfo->msgtime(tbNow.time);
				char sqlBuffer[1024];
				memset(sqlBuffer, 0, sizeof(sqlBuffer));
				if (dialogInfo.get() == NULL)
				{
					dialogInfo = CDialogInfo::create(dialogId, fromUserInfo);
					m_dialogs.remove(dialogId);
					m_dialogs.insert(dialogId, dialogInfo);

					m_avs2.remove(dialogId);
					CAccountConversation::pointer accountConversation = CAccountConversation::create(CFromInfo::create(dialogInfo));
					m_avs2.insert(dialogId, accountConversation);

					conversationInfo = CConversationInfo::create(CFromInfo::create(dialogInfo), memberUserInfo, messageInfo, CConversationInfo::CT_CREATE);
					sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromid,msgtype,subtype,fromaccount, sayaccount)\
									   VALUES(%d,%ld,601,%d,'%s','%s')",
									   (int)CFromInfo::FromDialogInfo, dialogId, (int)CConversationInfo::CT_CREATE,
									   fromAccount.c_str(), fromAccount.c_str());
				}else
				{
					if (dialogInfo->m_members.exist(memberUserAccount))
					{
						continue;
					}

					conversationInfo = CConversationInfo::create(CFromInfo::create(dialogInfo), memberUserInfo, messageInfo, CConversationInfo::CT_INVITE);
					sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromid,msgtype,subtype,fromaccount,sayaccount)\
									   VALUES(%d,%ld,601,%d,'%s','%s')",
									   (int)CFromInfo::FromDialogInfo, dialogId, (int)CConversationInfo::CT_INVITE,
									   fromAccount.c_str(), memberUserAccount.c_str());
				}
				conversationInfo->fromAccount(fromUserInfo);
				m_unreads.add(conversationInfo);
				bodb_exec(sqlBuffer);

				dialogInfo->m_members.insert(memberUserInfo->getAccount(), memberUserInfo);

				if (!ownerMemberAccount)
				{
					m_handler->onDialogInvited(dialogInfo, memberUserInfo);
				}
			}

		}break;
	case const_CallSign_DiaInvite:
		{
			if (m_account.get() == NULL) break;

			long dialogId = response.getRecvParameterValue(_T("DID"), 0);
			if (dialogId == 0)
			{
				break;
			}

			// ??
			m_dialogs.remove(dialogId);
			CDialogInfo::pointer dialogInfo = CDialogInfo::create(dialogId, m_account->getUserinfo());
			m_dialogs.insert(dialogId, dialogInfo);

			// ??
			m_avs2.remove(dialogId);
			CAccountConversation::pointer accountConversation = CAccountConversation::create(CFromInfo::create(dialogInfo));
			m_avs2.insert(dialogId, accountConversation);

			char sqlBuffer[1024];
			memset(sqlBuffer, 0, sizeof(sqlBuffer));
			sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromid,msgtype,subtype,fromaccount,sayaccount)\
							   VALUES(%d,%ld,601,%d,'%s','%s')",
							   (int)CFromInfo::FromDialogInfo, dialogId, (int)CConversationInfo::CT_CREATE,
							   m_account->getUserinfo()->getAccount().c_str(), m_account->getUserinfo()->getAccount().c_str());
			bodb_exec(sqlBuffer);
		}break;
	case const_CallSign_MsgRequest:
		{
			if (m_account.get() == NULL) break;

			long mid = response.getRecvParameterValue(_T("MID"), 0);
			long newmid = response.getRecvParameterValue(_T("NMID"), 0);
			CSendInfo::pointer sendInfo;
			if (!m_sends.find(mid, sendInfo, true))
			{
				break;
			}
			sendInfo->msginfo()->messageid(newmid);

			CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(sendInfo->fromUser()), m_account->getUserinfo(), sendInfo->msginfo());
			m_unreads.add(conversationInfo);

			if (sendInfo->msginfo()->type() == 13)
			{
				m_recvs.insert(newmid, sendInfo);
				break;
			}
			m_sends.insert(newmid, sendInfo);

			// Create the send thread.
			boost::thread * sendThread = new boost::thread(boost::bind(doStaProcSend, this, sendInfo));
			m_sendthreads.insert(newmid, sendThread);
		}break;
	case 512:
		{
			// MsgResponse event from Server.
			const tstring & fromAccount = response.getRecvParameterValue(_T("FromAccount"));
			long mid = response.getRecvParameterValue(_T("MID"), 0);
			if (mid == 0) break;
			long nResponse = response.getRecvParameterValue(_T("Response"), 0);

			CSendInfo::pointer sendInfo;
			if (nResponse == 3)
			{
				if (!m_recvs.find(mid, sendInfo, true))
				{
					break;
				}

				switch (sendInfo->msginfo()->type())
				{
				case 13:
					{
						closeLocalAV();
						closeRemoteAV();
					}break;
				default:
					{
						std::string filename = sendInfo->filename().empty() ? sendInfo->msginfo()->tostring() : sendInfo->filename();
						// Save to db.
						char sqlBuffer[10*1024];
						memset(sqlBuffer, 0, sizeof(sqlBuffer));
						sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,subtype,sayaccount,message,msgsize,newflag)\
										   VALUES(%d,'%s',11,%d,'%s','%s',%d,true)",
										   (int)CFromInfo::FromUserInfo, fromAccount.c_str(), (int)CConversationInfo::CT_CANCEL,
										   fromAccount.c_str(), filename.c_str(),
										   sendInfo->msginfo()->filesize());
						bodb_exec(sqlBuffer);

						sendInfo->cancel();

					}break;
				}
				CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(sendInfo->fromUser()), sendInfo->fromUser(), sendInfo->msginfo(), CConversationInfo::CT_CANCEL);
				m_unreads.add(conversationInfo);
			}else
			{
				if (!m_sends.find(mid, sendInfo, nResponse != 1))
				{
					break;
				}


				CConversationInfo::ConversationType conversationType;
				bool bAccept = nResponse == 1;
				if (nResponse == 1)
				{
					conversationType = CConversationInfo::CT_ACCEPT;
				}else if (nResponse == 2)
				{
					conversationType = CConversationInfo::CT_REJECT;
				}else
				{
					// error
					break;
				}

				// Save to db.
				char sqlBuffer[10*1024];
				memset(sqlBuffer, 0, sizeof(sqlBuffer));
				sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,subtype,sayaccount,message,msgsize,newflag)\
								   VALUES(%d,'%s',11,%d,'%s','%s',%d,true)",
								   (int)CFromInfo::FromUserInfo, fromAccount.c_str(), conversationType,
								   fromAccount.c_str(), sendInfo->msginfo()->tostring().c_str(),
								   sendInfo->msginfo()->filesize());
				bodb_exec(sqlBuffer);
				sendInfo->accept(bAccept);
				CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(sendInfo->fromUser()), sendInfo->fromUser(), sendInfo->msginfo(), conversationType);
				m_unreads.add(conversationInfo);
			}

			if (nResponse != 1)
			{
				boost::thread * sendThread = m_sendthreads.find(mid, true);
				if (sendThread != NULL)
				{
					sendThread->join();
					delete sendThread;
				}
			}

		}break;
	case 511:
		{
			// MsgRequest event from Server.
			const tstring & fromAccount = response.getRecvParameterValue(_T("FromAccount"));
			long mid = response.getRecvParameterValue(_T("MID"), 0);
			if (mid == 0) break;
			long type = response.getRecvParameterValue(_T("Type"), 0);

			CUserInfo::pointer fromUserInfo = getUserInfo(fromAccount);
			if (fromUserInfo.get() == 0) break;

			switch (type)
			{
			case 11:
				{
					// send file
					const tstring & sFilename = response.getRecvParameterValue(_T("Name"));
					if (sFilename.empty()) break;
					long nFilesize = response.getRecvParameterValue(_T("Size"), 0);
					if (nFilesize <= 0) break;

					// Save to db.
					char sqlBuffer[10*1024];
					memset(sqlBuffer, 0, sizeof(sqlBuffer));
					sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,sayaccount,message,msgsize,newflag)\
									   VALUES(%d,'%s',11,'%s','%s',%d,true)",
									   (int)CFromInfo::FromUserInfo, fromAccount.c_str(), 
									   fromAccount.c_str(), sFilename.c_str(),
									   nFilesize);
					bodb_exec(sqlBuffer);

					CMessageInfo::pointer messageInfo = CMessageInfo::create(mid, (unsigned long)sFilename.size(),  11, true);
					messageInfo->setdata(sFilename.c_str(), (unsigned long)sFilename.size(), 0);
					messageInfo->filesize(nFilesize);
					m_recvs.insert(messageInfo->messageid(), CSendInfo::create(fromUserInfo, messageInfo));

					CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(fromUserInfo), fromUserInfo, messageInfo);
					conversationInfo->readState(false);
					m_unreads.add(conversationInfo);

					// 不能直接调用事件
					//m_handler->onSendFileRequest(fromUserInfo, mid, sFilename, nFilesize);
				}break;
			case 13:
				{
					// video call
					CMessageInfo::pointer messageInfo = CMessageInfo::create(mid, 0,  (short)type, true);
					m_recvs.insert(messageInfo->messageid(), CSendInfo::create(fromUserInfo, messageInfo));

					CConversationInfo::pointer conversationInfo = CConversationInfo::create(CFromInfo::create(fromUserInfo), fromUserInfo, messageInfo);
					conversationInfo->readState(false);
					m_unreads.add(conversationInfo);

				}break;
			default:
				break;
			}

		}break;
	case 501:
		{
			// MsgSend Event From P2P(or server)
			if (m_account.get() == 0) break;
			if (!response.isRecvHasAttachInfo()) break;

			long dialogId = response.getRecvParameterValue(_T("DID"), 0);
			CDialogInfo::pointer dialogInfo;
			if (dialogId != 0)
			{
				dialogInfo = getDialogInfo(dialogId);
				if (dialogInfo.get() == NULL)
				{
					break;
				}
			}
			const tstring & fromAccount = response.getRecvParameterValue(_T("FromAccount"));
			long mid = response.getRecvParameterValue(_T("MID"), 0);
			if (mid == 0) break;

			cgcAttachment::pointer attach = response.getRecvAttachment();
			BOOST_ASSERT (attach.get() != NULL);

			CMessageInfo::pointer messageInfo;
			m_messageinfos.find(mid, messageInfo);

			if (messageInfo.get() != NULL)
			{
				if (m_rejects.exist(mid))
				{
					break;
				}
				bool errorMessageInfo = false;
				switch (messageInfo->type())
				{
				case 1:
				case 3:
					errorMessageInfo = messageInfo->total() != attach->getTotal();
					break;
				case 11:
					errorMessageInfo = messageInfo->filesize() != attach->getTotal();
					break;
				default:
					break;
				}
				if (errorMessageInfo)
				{
					m_messageinfos.remove(mid);
					messageInfo.reset();
				}
			}
			if (attach->getTotal() == 0 || attach->getAttachSize() == 0)
			{
				// error
				break;
			}

			if (messageInfo.get() == NULL)
			{
				if (attach->getIndex() == 0)
				{
					long type = response.getRecvParameterValue(_T("Type"), 0);
					bool bNew = response.getRecvParameterValue2(_T("New"), true);

					struct timeb tbNow;
					ftime(&tbNow);
					tbNow.time -= (tbNow.timezone*60);
					time_t msgTime = (time_t)response.getRecvParameterValue(_T("Time"), (long)tbNow.time);

					switch (type)
					{
					case 1:
						messageInfo = CMessageInfo::create(mid, attach->getTotal(), (short)type, bNew);
						break;
					case 3:
						{
							long imageWidth = response.getRecvParameterValue(_T("Width"), 0);
							long imageHeight = response.getRecvParameterValue(_T("Height"), 0);
							messageInfo = CMessageInfo::create(mid, attach->getTotal(), (short)type, bNew);
							messageInfo->imageWH(imageWidth, imageHeight);
						}break;
					case 11:
						{
							CSendInfo::pointer sendInfo;
							if (!m_recvs.find(mid, sendInfo))
							{
								// error
								return;
							}

							messageInfo = sendInfo->msginfo();
						}break;
					default:
						// error
						return;
						break;
					}
					messageInfo->msgtime(msgTime);
				}else
				{
					CSendInfo::pointer sendInfo;
					if (m_recvs.find(mid, sendInfo))
					{
						messageInfo = sendInfo->msginfo();
					}else
					{
						messageInfo = CMessageInfo::create(mid, attach->getTotal());
					}
				}

				size_t indexCount = 0;
				if (attach->getIndex() + attach->getAttachSize() == attach->getTotal())
				{
					indexCount = 1;
					if (attach->getIndex() > 0)
					{
						indexCount++;
						messageInfo->m_indexs.insert(attach->getIndex(), true);
					}
					//messageInfo->m_indexs.insert(attach->getAttachSize(), true);
					messageInfo->m_indexs.insert(attach->getTotal(), true);
				}else
				{
					indexCount = attach->getTotal() / attach->getAttachSize();
					if (attach->getTotal() % attach->getAttachSize() > 0)
						indexCount += 1;

					for (size_t i=1; i<= indexCount; i++)
					{
						unsigned long index = (i==indexCount) ? attach->getTotal() : i*attach->getAttachSize();
						messageInfo->m_indexs.insert(index, true);
					}
				}

				messageInfo->indexCount(indexCount);
				m_messageinfos.insert(mid, messageInfo);
			}else if (attach->getIndex() == 0)
			{
				long type = response.getRecvParameterValue(_T("Type"), 0);
				bool bNew = response.getRecvParameterValue2(_T("New"), true);

				struct timeb tbNow;
				ftime(&tbNow);
				tbNow.time -= (tbNow.timezone*60);
				time_t msgTime = (time_t)response.getRecvParameterValue(_T("Time"), (long)tbNow.time);

				messageInfo->type((short)type);
				messageInfo->newflag(bNew);
				messageInfo->msgtime(msgTime);
				if (type == 3)
				{
					long imageWidth = response.getRecvParameterValue(_T("Width"), 0);
					long imageHeight = response.getRecvParameterValue(_T("Height"), 0);
					messageInfo->imageWH(imageWidth, imageHeight);
				}
			}
			switch (messageInfo->type())
			{
			case 1:
			case 3:
				messageInfo->setdata((const char*)attach->getAttachData(), attach->getAttachSize(), attach->getIndex());
				break;
			case 11:
				{
					CSendInfo::pointer sendInfo;
					if (m_recvs.find(mid, sendInfo))
					{
						tfstream & filestream = sendInfo->fs();
						if (filestream.is_open())
						{
							filestream.seekp(attach->getIndex(), std::ios::beg);
							filestream.write((const char*)attach->getAttachData(), attach->getAttachSize());
							filestream.flush();
						}
					}
				}break;
			default:
				break;
			}
			messageInfo->m_indexs.remove(attach->getIndex() + attach->getAttachSize());
			if (!messageInfo->m_indexs.empty())
			{
				// Already had data.
				switch (messageInfo->type())
				{
				case 11:	// file
					{
						size_t indexCount = messageInfo->indexCount();

						if (indexCount-messageInfo->m_indexs.size() == 1)
						{
							CUserInfo::pointer fromUserInfo = getUserInfo(fromAccount);
							if (fromUserInfo.get() == NULL) break;

							CFromInfo::pointer fromInfo = dialogInfo.get() != NULL ? CFromInfo::create(dialogInfo) : CFromInfo::create(fromUserInfo);
							CConversationInfo::pointer conversationInfo = CConversationInfo::create(fromInfo, fromUserInfo, messageInfo);
							conversationInfo->readState(false);
							conversationInfo->percent(0.00);
							m_unreads.add(conversationInfo);
						}else
						{
							size_t eventCount = indexCount / 80;
							eventCount = eventCount < 20 ? 20 : eventCount;
							//static size_t receiveCount = 0;

							//if (++receiveCount == eventCount)
							if (messageInfo->icounter() == eventCount)
							{
								messageInfo->resetcounter();
								//receiveCount = 0;
								CUserInfo::pointer fromUserInfo = getUserInfo(fromAccount);
								if (fromUserInfo.get() == NULL) break;

								CFromInfo::pointer fromInfo = dialogInfo.get() != NULL ? CFromInfo::create(dialogInfo) : CFromInfo::create(fromUserInfo);
								CConversationInfo::pointer conversationInfo = CConversationInfo::create(fromInfo, fromUserInfo, messageInfo);
								conversationInfo->readState(false);
								conversationInfo->percent((float)(indexCount-messageInfo->m_indexs.size()) * 100.0 / indexCount);
								m_unreads.add(conversationInfo);
							}
						}
					}break;
				default:
					break;
				}

				break;
			}
			m_messageinfos.remove(mid);

			//CCompanyInfoPointer companyInfo;
			//CCoGroupInfo::pointer cogroupInfo;
			//CUserInfo::pointer fromUserInfo = getUserInfo(fromAccount, companyInfo, cogroupInfo);
			CUserInfo::pointer fromUserInfo = getUserInfo(fromAccount);
			if (fromUserInfo.get() != 0)
			{
				CFromInfo::pointer fromInfo = dialogInfo.get() != NULL ? CFromInfo::create(dialogInfo) : CFromInfo::create(fromUserInfo);

				bool bExistConversation = false;
				if (fromInfo->fromType() == CFromInfo::FromUserInfo)
				{
					bExistConversation = m_avs.exist(fromInfo->fromUser()->getAccount());
				}else if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
				{
					bExistConversation = m_avs2.exist(fromInfo->fromDialog()->dialogId());
				}

				if (!bExistConversation)
				{
					// ?
					getAccountConversations(fromInfo);
				}

				if (fromInfo->fromType() == CFromInfo::FromDialogInfo && dialogInfo.get() == NULL)
				{
					// ???

				}

				switch (messageInfo->type())
				{
				case 1:
					{
						CConversationInfo::pointer conversationInfo = CConversationInfo::create(fromInfo, fromUserInfo, messageInfo);
						conversationInfo->readState(false);
						m_unreads.add(conversationInfo);

						//CAccountConvarsation::
						char sqlBuffer[10*1024];
						memset(sqlBuffer, 0, sizeof(sqlBuffer));
						if (fromInfo->fromType() == CFromInfo::FromUserInfo)
						{
							sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,sayaccount,message,msgsize,newflag)\
											   VALUES(%d,'%s',1,'%s','%s',%d,%s)",
											   (int)CFromInfo::FromUserInfo, fromAccount.c_str(), fromAccount.c_str(), messageInfo->getdata(),
											   messageInfo->total(), messageInfo->newflag() ? "true" : "false");
						}else if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
						{
							sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromid,msgtype,sayaccount,message,msgsize,newflag)\
											   VALUES(%d,%ld,1,'%s','%s',%d,%s)",
											   (int)CFromInfo::FromDialogInfo, fromInfo->fromDialog()->dialogId(), fromAccount.c_str(),
											   messageInfo->getdata(),messageInfo->total(), messageInfo->newflag() ? "true" : "false");
						}
						bodb_exec(sqlBuffer);
					}break;
				case 3:
					{
						// Save the image
						char filename[50];
						sprintf(filename, "%I64d%08d", time(0), mid);
						char filepath[256];
						sprintf(filepath, "%s/users/%s/File/%s", this->m_sModulePath.c_str(), m_account->getUserinfo()->getAccount().c_str(), filename);
						FILE * hfile = fopen(filepath, "wb");
						if (hfile != NULL)
						{
							fwrite(messageInfo->getdata(), 1, messageInfo->total(), hfile);
							fclose(hfile);
						}
						// Save to db.
						char sqlBuffer[10*1024];
						memset(sqlBuffer, 0, sizeof(sqlBuffer));
						if (fromInfo->fromType() == CFromInfo::FromUserInfo)
						{
							sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,sayaccount,message,msgsize,width,height,newflag)\
											   VALUES(%d,'%s',3,'%s','%s',%d,%d,%d,%s)",
											   (int)CFromInfo::FromUserInfo, fromAccount.c_str(), fromAccount.c_str(), filename,
											   messageInfo->total(), messageInfo->imageWidth(), messageInfo->imageHeight(),
											   messageInfo->newflag() ? "true" : "false");
						}else if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
						{
							sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromid,msgtype,sayaccount,message,msgsize,width,height,newflag)\
											   VALUES(%d,%ld,3,'%s','%s',%d,%d,%d,%s)",
											   (int)CFromInfo::FromDialogInfo, fromInfo->fromDialog()->dialogId(), fromAccount.c_str(), filename,
											   messageInfo->total(), messageInfo->imageWidth(), messageInfo->imageHeight(),
											   messageInfo->newflag() ? "true" : "false");
						}
						bodb_exec(sqlBuffer);

						CAVCodecProxy m_videoCodec;
						FVInfo fvinfo(PIX_FMT_BGR24, 24, (int)messageInfo->imageWidth(), (int)messageInfo->imageHeight());
						m_videoCodec.setdec_info(fvinfo);
						if (!m_videoCodec.openav(CODEC_ID_MPEG4))
						{
							break;
						}
						unsigned char * decode_buffer;
						int decode_size = 0;
						if (!m_videoCodec.decode((const unsigned char*)messageInfo->getdata(), messageInfo->total(), &decode_buffer, decode_size, false))
						{
							break;
						}
						messageInfo->setdata((char*)decode_buffer, decode_size);

						CFromInfo::pointer fromInfo = dialogInfo.get() != NULL ? CFromInfo::create(dialogInfo) : CFromInfo::create(fromUserInfo);
						CConversationInfo::pointer conversationInfo = CConversationInfo::create(fromInfo, fromUserInfo, messageInfo);
						conversationInfo->readState(false);
						m_unreads.add(conversationInfo);
					}break;
				case 11:
					{
						CSendInfo::pointer sendInfo;
						if (!m_recvs.find(mid, sendInfo, true))
						{
							break;
						}

						// Save to db.
						char sqlBuffer[1024];
						memset(sqlBuffer, 0, sizeof(sqlBuffer));
						if (fromInfo->fromType() == CFromInfo::FromUserInfo)
						{
							sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromaccount,msgtype,subtype,sayaccount,message,msgsize,newflag)\
											   VALUES(%d,'%s',11,%d,'%s','%s',%d,true)",
											   (int)CFromInfo::FromUserInfo, sendInfo->fromUser()->getAccount().c_str(), (int)CConversationInfo::CT_SUCCEED,
											   sendInfo->fromUser()->getAccount().c_str(), sendInfo->filepath().c_str(),
											   sendInfo->msginfo()->filesize());
						}else if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
						{
							sprintf(sqlBuffer, "INSERT INTO accountconversations_t (fromtype,fromid,msgtype,subtype,sayaccount,message,msgsize,newflag)\
											   VALUES(%d,%ld,11,%d,'%s','%s',%d,true)",
											   (int)CFromInfo::FromDialogInfo, fromInfo->fromDialog()->dialogId(), (int)CConversationInfo::CT_SUCCEED,
											   sendInfo->fromUser()->getAccount().c_str(), sendInfo->filepath().c_str(),
											   sendInfo->msginfo()->filesize());
						}

						bodb_exec(sqlBuffer);

						messageInfo->resetdata(sendInfo->filepath().c_str(), (unsigned long)sendInfo->filepath().size());

						CConversationInfo::pointer conversationInfo = CConversationInfo::create(fromInfo, fromUserInfo, messageInfo, CConversationInfo::CT_SUCCEED);
						conversationInfo->readState(false);
						conversationInfo->percent(100.0);
						m_unreads.add(conversationInfo);
					}break;
				default:
					break;
				}
			}
		}break;
	case const_CallSign_FriGrpDestroy:
		{
			if (m_account.get() == 0) break;

			if (resultValue == 0)
			{
				int groupId = response.getRecvParameterValue(_T("Id"), 0);

				CGroupInfo::pointer groupInfo;
				if (m_account->m_allgroups.find(groupId, groupInfo, true))
				{
					m_handler->onFriendGroupDelete(groupInfo);
				}
			}else
			{
				m_handler->onFriendGroupDelete(resultValue);
			}
		}break;
	case const_CallSign_FriGrpSetName:
		{
			if (m_account.get() == 0) break;

			if (resultValue == 0)
			{
				int groupId = response.getRecvParameterValue(_T("Id"), 0);
				const tstring & groupName = response.getRecvParameterValue(_T("NewName"));

				CGroupInfo::pointer groupInfo;
				if (m_account->m_allgroups.find(groupId, groupInfo))
				{
					groupInfo->name(groupName);
					// ? event
				}
			}
		}break;
	case const_CallSign_FriLoad:
		{
			m_handler->onFriendLoadReturned(resultValue);
		}break;
	case const_CallSign_FriGrpCreate:
	case 101:
		{
			// Friend GroupInfo Event.

			if (m_account.get() == 0) break;

			int groupId = response.getRecvParameterValue(_T("Id"), 0);
			const tstring & groupName = response.getRecvParameterValue(_T("Name"));
			int parentId = response.getRecvParameterValue(_T("ParentId"), 0);
			int type = response.getRecvParameterValue(_T("Type"), 1);

			if (groupId > 0 && !groupName.empty())
			{
				CGroupInfo::pointer groupInfo = CGroupInfo::create(groupId, groupName, (CGroupInfo::GroupType)type);
				m_account->m_allgroups.insert(groupInfo->groupid(), groupInfo);

				CGroupInfo::pointer parentGroupInfo;
				if (parentId > 0 && parentId != groupId)
				{
					m_account->m_allgroups.find(parentId, parentGroupInfo);
				}
				groupInfo->parentgroup(parentGroupInfo);

				m_handler->onGroupInfo(response.getSign() == const_CallSign_FriGrpCreate, groupInfo);
			}
		}break;
	case 110:
		{
			// CompanyInfo event.
			if (m_account.get() == 0) break;

			int coId = response.getRecvParameterValue(_T("Id"), 0);
			const tstring & companyName = response.getRecvParameterValue(_T("Name"));

			if (coId > 0 && !companyName.empty())
			{
				CCompanyInfoPointer companyInfo;
				if (!m_account->getUserinfo()->m_companys.find(coId, companyInfo))
				{
					companyInfo = CCompanyInfo::create(coId, companyName);
					m_account->getUserinfo()->m_companys.insert(coId, companyInfo);
					m_handler->onCompanyInfo(companyInfo);
					//coLoadGroup(coId);
				}
			}
		}break;
	case const_CallSign_CoLoad:
		{
			// ?
			if (m_account.get() == 0) break;

			// ???
			//this->accountRegConfirm();
		}break;
	case const_CallSign_CoLoadGroup:
		{
			m_handler->onCoLoadGroupReturned(resultValue);
		}break;
	case 111:
		{
			// cogroupinfo event.
			if (m_account.get() == 0) break;

			int coId = response.getRecvParameterValue(_T("CoId"), 0);

			CCompanyInfoPointer companyInfo;
			if (!m_account->getUserinfo()->m_companys.find(coId, companyInfo))
			{
				break;
			}

			int groupId = response.getRecvParameterValue(_T("Id"), 0);
			const tstring & groupName = response.getRecvParameterValue(_T("Name"));
			int parentId = response.getRecvParameterValue(_T("ParentId"), 0);
			int type = response.getRecvParameterValue(_T("Type"), 1);
			bool isMyCoGroup = response.getRecvParameterValue2(_T("MyCoGroup"), false);

			if (groupId > 0 && !groupName.empty())
			{
				CCoGroupInfo::pointer cogroupInfo = CCoGroupInfo::create(companyInfo, groupId, groupName, (CCoGroupInfo::GroupType)type);
				companyInfo->m_allgroups.insert(cogroupInfo->groupid(), cogroupInfo);

				if (isMyCoGroup && !m_account->m_cogroups.exist(cogroupInfo->groupid()))
				{
					m_account->m_cogroups.insert(cogroupInfo->groupid(), cogroupInfo);
				}

				CCoGroupInfo::pointer parentGroupInfo;
				if (parentId > 0 && parentId != groupId)
				{
					companyInfo->m_allgroups.find(parentId, parentGroupInfo);
				}
				cogroupInfo->parentgroup(parentGroupInfo);

				m_handler->onCoGroupInfo(cogroupInfo, isMyCoGroup);
				// ?
				//if (parentGroupInfo.get() != NULL)
				//{
				//	this->coLoadGroupUser(coId, groupId);
				//}
			}

		}break;
	case const_CallSign_CoLoadGroupUser:
		break;
	case 112:
		{
			// CoGroup user event.
			if (m_account.get() == 0) break;

			int coId = response.getRecvParameterValue(_T("CoId"), 0);

			CCompanyInfoPointer companyInfo;
			if (!m_account->getUserinfo()->m_companys.find(coId, companyInfo))
			{
				break;
			}

			const tstring & account = response.getRecvParameterValue(_T("Account"));
			const tstring & name = response.getRecvParameterValue(_T("Name"));
			const tstring & nick = response.getRecvParameterValue(_T("Nick"));
			const tstring & phone = response.getRecvParameterValue(_T("Phone"));
			const tstring & email = response.getRecvParameterValue(_T("Email"));
			long lineState = response.getRecvParameterValue(_T("LineState"), 0);

			CUserInfoPointer userInfo;
			if (!companyInfo->m_userinfos.find(account, userInfo))
			{
				userInfo = CUserInfo::create(account, "");
				userInfo->setUserName(name);
				userInfo->setNick(nick);
				userInfo->setPhone(phone);
				userInfo->setEmail(email);
				userInfo->setLineState((CUserInfo::UserLineState)lineState);
				companyInfo->m_userinfos.insert(userInfo->getAccount(), userInfo);
			}

			int index = 0;
			while (true)
			{
				char buffer[10];
				sprintf(buffer, "Id%d", index++);
				int groupId = response.getRecvParameterValue(buffer, 0);
				if (groupId == 0)
				{
					break;
				}

				CCoGroupInfo::pointer cogroupInfo;
				if (!companyInfo->m_allgroups.find(groupId, cogroupInfo))
				{
					break;
				}

				if (!cogroupInfo->m_userinfos.exist(account))
				{
					if (!userInfo->isOfflineState())
					{
						companyInfo->iOnlineCount();
						cogroupInfo->iOnlineCount(true);
					}
					userInfo->m_cogroups.insert(cogroupInfo->groupid(), cogroupInfo);
					cogroupInfo->m_userinfos.insert(account, userInfo);

					bool isMyCoGroupUser = m_account->m_cogroups.exist(cogroupInfo->groupid());
					m_handler->onCoGroupUser(cogroupInfo, userInfo, isMyCoGroupUser);
				}
			}
		}break;
	case 201:
		{
			// User login notify
			const tstring & fromAccount = response.getRecvParameterValue(_T("FromAccount"));

			CFriendInfo::pointer friendInfo;
			if (m_account->m_allfriends.find(fromAccount, friendInfo))
			{
				friendInfo->userinfo()->setLineState(CUserInfo::UserOnLineState);
				m_handler->onUserLogined(friendInfo);
			}else
			{
				CLockMap<unsigned int, CCompanyInfoPointer>::const_iterator iterCompany;
				boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(m_account->getUserinfo()->m_companys.mutex()));
				for (iterCompany=m_account->getUserinfo()->m_companys.begin(); iterCompany!=m_account->getUserinfo()->m_companys.end(); iterCompany++)
				{
					CCompanyInfoPointer companyInfo = iterCompany->second;
					companyInfo->iOnlineCount();

					CLockMap<unsigned int, CCoGroupInfo::pointer>::const_iterator iterCoGroup;
					//boost::mutex::scoped_lock lockCoGroupInfo(const_cast<boost::mutex&>(companyInfo->m_allgroups.mutex()));
					for (iterCoGroup=companyInfo->m_allgroups.begin(); iterCoGroup!=companyInfo->m_allgroups.end(); iterCoGroup++)
					{
						CCoGroupInfo::pointer cogroupInfo = iterCoGroup->second;
						CUserInfo::pointer fromUserInfo;
						if (cogroupInfo->m_userinfos.find(fromAccount, fromUserInfo))
						{
							cogroupInfo->iOnlineCount(false);
							fromUserInfo->setLineState(CUserInfo::UserOnLineState);
							m_handler->onUserLogined(cogroupInfo, fromUserInfo);

						}
					}
				}
			}

		}break;
	case 202:
		{
			if (m_account.get() == NULL) break;

			// User logout notify
			const tstring & fromAccount = response.getRecvParameterValue(_T("FromAccount"));

			if (m_account->getUserinfo()->getAccount() == fromAccount)
			{
				bool serverQuit = response.getRecvParameterValue2(_T("Quit"), false);

				CbiwooHandler::LogoutType logoutType = serverQuit ? CbiwooHandler::ServerQuit : CbiwooHandler::LT_LoginAnotherPlace;
				m_p2pav.AccountLogout();
				m_handler->onUserLogouted(m_account, logoutType);
				m_account.reset();
				break;
			}

			CFriendInfo::pointer friendInfo;
			if (m_account->m_allfriends.find(fromAccount, friendInfo))
			{
				friendInfo->userinfo()->setLineState(CUserInfo::UserOffLineState);
				m_handler->onUserLogouted(friendInfo);
			}else
			{
				/////////////////////////////////
				// Response
				CLockMap<unsigned int, CCompanyInfoPointer>::const_iterator iterCompany;
				boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(m_account->getUserinfo()->m_companys.mutex()));
				for (iterCompany=m_account->getUserinfo()->m_companys.begin(); iterCompany!=m_account->getUserinfo()->m_companys.end(); iterCompany++)
				{
					CCompanyInfoPointer companyInfo = iterCompany->second;
					companyInfo->dOnlineCount();

					CLockMap<unsigned int, CCoGroupInfo::pointer>::const_iterator iterCoGroup;
					//boost::mutex::scoped_lock lockCoGroupInfo(const_cast<boost::mutex&>(companyInfo->m_allgroups.mutex()));
					for (iterCoGroup=companyInfo->m_allgroups.begin(); iterCoGroup!=companyInfo->m_allgroups.end(); iterCoGroup++)
					{
						CCoGroupInfo::pointer cogroupInfo = iterCoGroup->second;
						CUserInfo::pointer fromUserInfo;
						if (cogroupInfo->m_userinfos.find(fromAccount, fromUserInfo))
						{
							cogroupInfo->dOnlineCount(false);
							fromUserInfo->setLineState(CUserInfo::UserOffLineState);
							m_handler->onUserLogouted(cogroupInfo, fromUserInfo);
						}
					}
				}
			}
		}break;
	case const_CallSign_Register:
		{
			// 登录成功
			if (resultValue == 0)
			{
				if (m_account.get() != 0)
					m_account.reset();

				const tstring & sAccount = response.getRecvParameterValue(_T("Account"));
				const tstring & sUserName = response.getRecvParameterValue(_T("Name"));
				const tstring & sAccountId = response.getRecvParameterValue(_T("AccountId"));

				if (!m_p2pav.AccountLogin(sAccount))
				{
					break;
				}

				std::string sPath = m_sModulePath;
				sPath.append("/users/");

				namespace fs = boost::filesystem;
				fs::path pathRoot(sPath, fs::native);
				if (!boost::filesystem::exists(sPath))
				{
					fs::create_directory(sPath);
				}
				std::string sPathUser = sPath + sAccount;
				fs::path pathUser(sPathUser, fs::native);
				if (!boost::filesystem::exists(pathUser))
				{
					////////////// New account.
					// Create account directory.
					fs::create_directory(pathUser);

					// Create File directory
					std::string sPathUserFile(sPathUser);
					sPathUserFile.append("/File");
					fs::path pathUserFile(sPathUserFile, fs::native);
					fs::create_directory(pathUserFile);

					// Create user database directory.
					std::string sPathUserdb(sPathUser);
					sPathUserdb.append("/bocdb");
					fs::path pathUserdb(sPathUserdb, fs::native);
					fs::create_directory(sPathUserdb);

					// Copy the default database file.
					std::string sDefaultDBFile(m_sModulePath);
					sDefaultDBFile.append("/default/bocdb/bocdb.bdf");
					sPathUserdb.append("/bocdb.bdf");
					fs::copy_file(fs::path(sDefaultDBFile, fs::native), fs::path(sPathUserdb, fs::native));
				}

				CUserInfo::pointer userinfo = CUserInfo::create(sAccount, _T(""));
				userinfo->setUserName(sUserName);
				m_account = CAccountInfo::create(userinfo, sAccountId);
				m_handler->onUserLogined(m_account);
			}else
			{
				m_handler->onUserLoginError(resultValue);
			}
			return;
		}break;
	case const_CallSign_RegConfirm:
		{
			BOOST_ASSERT (m_account.get() != NULL);

			std::string sPathUser(m_sModulePath);
			sPathUser.append("/users/");
			sPathUser.append(m_account->getUserinfo()->getAccount());
			if (!bodb_init(sPathUser.c_str()))
			{
				break;
			}

			std::string sql("USE bocdb");
			if (bodb_exec(sql.c_str()) != 0)
			{
				break;
			}

			// Select conversations from bocdb database.
			char sqlBuffer[1024];
			memset(sqlBuffer, 0, sizeof(sqlBuffer));
			sprintf(sqlBuffer, "SELECT DISTINCT fromtype,fromid,fromaccount,sayaccount,msgtype,subtype FROM accountconversations_t");
			PRESULTSET resultset = 0;
			bodb_exec(sqlBuffer, &resultset);
			if (resultset != 0)
			{
				for (int i=0; i<resultset->rscount; i++)
				{
					CFieldVariant varFromType(resultset->rsvalues[i]->fieldvalues[0]);
					CFieldVariant varFromId(resultset->rsvalues[i]->fieldvalues[1]);
					CFieldVariant varFromAccount(resultset->rsvalues[i]->fieldvalues[2]);

					int nFromType = varFromType.getInt();
					int nFromId = varFromId.getInt();
					std::string sFromAccount = varFromAccount.getString();
					CUserInfo::pointer fromUserInfo = m_account->getUserinfo()->getAccount() == sFromAccount ? m_account->getUserinfo() : getUserInfo(sFromAccount);
					if (fromUserInfo.get() == NULL)
					{
						continue;
					}

					if (nFromType == (int)CFromInfo::FromUserInfo)
					{
						m_recents.add(CFromInfo::create(fromUserInfo));
					}else if (nFromType == (int)CFromInfo::FromDialogInfo)
					{
						CFieldVariant varSayAccount(resultset->rsvalues[i]->fieldvalues[3]);
						std::string sSayAccount = varSayAccount.getString();
						CUserInfo::pointer sayUserInfo = m_account->getUserinfo()->getAccount() == sSayAccount ? m_account->getUserinfo() : getUserInfo(sSayAccount);
						if (sayUserInfo.get() == NULL)
						{
							continue;
						}

						CFieldVariant varMsgType(resultset->rsvalues[i]->fieldvalues[4]);
						CFieldVariant varSubType(resultset->rsvalues[i]->fieldvalues[5]);

						int nMsgType = varMsgType.getInt();
						int nSubType = varSubType.getInt();

						if (nMsgType == 601)
						{
							if (varSubType.getInt() == CConversationInfo::CT_CREATE)
							{
								CDialogInfo::pointer dialogInfo = CDialogInfo::create((long)nFromId, fromUserInfo);
								dialogInfo->history(true);
								this->m_dialogs.insert(dialogInfo->dialogId(), dialogInfo);

								if (fromUserInfo->getAccount() != m_account->getUserinfo()->getAccount())
								{
									dialogInfo->m_members.insert(fromUserInfo->getAccount(), fromUserInfo);
								}

								m_recents.add(CFromInfo::create(dialogInfo));
							}else if (varSubType.getInt() == CConversationInfo::CT_INVITE)
							{
								CDialogInfo::pointer dialogInfo;
								if (m_dialogs.find((long)nFromId, dialogInfo))
								{
									dialogInfo->m_members.insert(sayUserInfo->getAccount(), sayUserInfo);
								}
							}
						}else if (nMsgType == 602)
						{
							if (varSubType.getInt() == CConversationInfo::CT_QUIT)
							{
								CDialogInfo::pointer dialogInfo;
								if (m_dialogs.find((long)nFromId, dialogInfo))
								{
									dialogInfo->m_members.remove(sayUserInfo->getAccount());
								}
							}
						}else
						{
							CDialogInfo::pointer dialogInfo = getDialogInfo((long)nFromId);
							if (dialogInfo.get() != NULL)
							{
								m_recents.add(CFromInfo::create(dialogInfo));
							}
						}
					}else
					{
						//m_recents.add(CFromInfo::create(fromUserInfo));
					}
				}

				bodb_free(resultset);
			}
			m_handler->onRegConfirm();
		}break;
	case const_CallSign_UnRegister:
		{
			if (m_account.get() != 0)
			{
				m_dialogs.clear();
				m_avs.clear();
				m_avs2.clear();
				m_messageinfos.clear();
				m_rejects.clear();
				m_unreads.clear();
				m_recents.clear();
				bodb_exit();
				m_p2pav.AccountLogout();
				m_handler->onUserLogouted(m_account, CbiwooHandler::LT_Normal);
				m_account.reset();
			}
		}break;
	case const_CallSign_AccLoad:
		{
			this->accountRegConfirm();
		}break;
	case const_CallSign_GetAllUser:
		{
			cgcParameter::pointer pUserId = response.getRecvParameter(_T("UserId"));
			cgcParameter::pointer pUserName = response.getRecvParameter(_T("FromUser"));
			if (pUserId.get() != NULL && pUserName.get() != NULL)
			{
				m_handler->onUserInfo(pUserId->getValue(), pUserName->getValue());
			}
		}break;
	default:
		{
		}break;
	}

}
