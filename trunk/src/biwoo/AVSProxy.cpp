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

#include "AVSProxy.h"
#include <bodb/bodb2.h>
#include <bodb/fieldvariant.h>
using namespace bo;
#include <CGCBase/includeapp.h>
#include <sys/timeb.h>


CAVSProxy gAVSProxy;

void CAVSProxy::deleteDbOffEvent(unsigned long id)
{
	// Delete offevent
	if (id > 0 && bodb_isopened())
	{
		char sql[1024];
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "DELETE FROM offevents_t WHERE id=%ld", id);
		bodb_exec(sql);
	}
}

void CAVSProxy::addOffEvent(CUserInfo::pointer touserInfo, COfflineEvent::pointer offlineEvent)
{
	BOOST_ASSERT (touserInfo.get() != 0);
	BOOST_ASSERT (offlineEvent.get() != 0);
	BOOST_ASSERT (offlineEvent->fromInfo().get() != 0);

	touserInfo->m_offevents.add(offlineEvent);

	CMessageInfo::pointer messageInfo = offlineEvent->getMessage();
	if (messageInfo.get() == 0)
	{
		return;
	}

	struct timeb tbNow;
	ftime(&tbNow);
	tbNow.time -= (tbNow.timezone*60);
	offlineEvent->getMessage()->msgtime(tbNow.time);

	if (bodb_isopened())
	{
		long nFromId = 0;
		if (offlineEvent->fromInfo()->fromType() == CFromInfo::FromDialogInfo)
		{
			nFromId = offlineEvent->fromInfo()->fromDialog()->dialogId();
		}
		char sql[10*1024];
		memset(sql, 0, sizeof(sql));
		switch (offlineEvent->getEvent())
		{
		case 601:	// diainvite
		case 602:	// diaquit
			{
				sprintf(sql, "INSERT INTO offevents_t (event,fromtype,fromid,fromaccount,toaccount,message,msgtype) \
							 VALUES(%d,%d,%ld,'%s','%s','%s',%d)",
							 offlineEvent->getEvent(), (int)offlineEvent->fromInfo()->fromType(), nFromId, offlineEvent->getFromAccount()->getAccount().c_str(),
							 touserInfo->getAccount().c_str(), offlineEvent->toAccount()->getAccount().c_str(), messageInfo->type()
							 );
			}break;
		default:
			{
				switch (messageInfo->type())
				{
				case 3:
					{
						// image
						sprintf(sql, "INSERT INTO offevents_t (event,fromtype,fromid,fromaccount,toaccount,msgtype,newflag,message,msgsize,width,height) \
									 VALUES(%d,%d,%ld,'%s','%s',%d, %s, '%s', %d, %d, %d)",
									 offlineEvent->getEvent(), (int)offlineEvent->fromInfo()->fromType(), nFromId, offlineEvent->getFromAccount()->getAccount().c_str(),
									 touserInfo->getAccount().c_str(), messageInfo->type(),
									 messageInfo->newflag() ? "true" : "false",
									 messageInfo->getdata(), messageInfo->filesize(),
									 messageInfo->imageWidth(), messageInfo->imageHeight()
									 );
					}break;
				case 11:
					{
						// ??
						// Do not support to save the file offevent.
						return;
					}break;
				default:
					{

						sprintf(sql, "INSERT INTO offevents_t (event,fromtype,fromid,fromaccount,toaccount,msgtype,newflag,message,msgsize)\
									 VALUES(%d,%d,%ld,'%s','%s',%d, %s, '%s', %ld)",
									 offlineEvent->getEvent(), (int)offlineEvent->fromInfo()->fromType(), nFromId, offlineEvent->getFromAccount()->getAccount().c_str(),
									 touserInfo->getAccount().c_str(), messageInfo->type(),
									 messageInfo->newflag() ? "true" : "false",
									 messageInfo->getdata(), messageInfo->total()
									 );
					}break;
				}
			}break;
		}

		bodb_exec(sql);

	}
}

void CAVSProxy::addUserinfo(CUserInfo::pointer userInfo)
{
	BOOST_ASSERT (userInfo.get() != 0);
	//m_users.insert(userInfo->getAccount(), userInfo);
	gApplication->setAttribute(BMT_ALLUSERS, userInfo->getAccount(), userInfo);

	if (bodb_isopened())
	{
		char sql[2048];
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "INSERT INTO userinfo_t (account,password,username,nick,gender,birthday,phone,email)\
					 VALUES('%s','%s','%s','%s',%d,'%s','%s','%s')",
			userInfo->getAccount().c_str(), userInfo->getPassword().c_str(),
			userInfo->getUserName().c_str(), userInfo->getNick().c_str(),
			userInfo->getGender(), userInfo->getBirthday().c_str(),
			userInfo->getPhone().c_str(), userInfo->getEmail().c_str());
		bodb_exec(sql);
	}
}

void CAVSProxy::updateUserinfo(CUserInfo::pointer userInfo)
{
	if (bodb_isopened())
	{
		char sql[2048];
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "UPDATE userinfo_t SET nick='%s',gender=%d,phone='%s',email='%s' WHERE account='%s'",
			userInfo->getNick().c_str(), userInfo->getGender(), 
			userInfo->getPhone().c_str(), userInfo->getEmail().c_str(), userInfo->getAccount().c_str());
		bodb_exec(sql);
	}
}

void CAVSProxy::deleteUserinfo(const tstring & sAccount)
{
	//m_users.remove(sAccount);
	gApplication->removeAttribute(BMT_ALLUSERS, sAccount);

	if (bodb_isopened())
	{
		char sql[2048];
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "DELETE FROM userinfo_t WHERE account='%s'", sAccount.c_str());
		bodb_exec(sql);
	}
}

void CAVSProxy::updatePassword(const tstring & sAccount, const tstring & sNewPassword)
{
	if (bodb_isopened())
	{
		char sql[1024];
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "UPDATE userinfo_t SET password='%s' WHERE account='%s'", sNewPassword.c_str(), sAccount.c_str());
		bodb_exec(sql);
	}
}

void CAVSProxy::loadAccountInfo(CAccountInfo::pointer accountInfo)
{
	BOOST_ASSERT (accountInfo.get() != 0);
	BOOST_ASSERT (accountInfo->getUserinfo().get() != 0);

	if (bodb_isopened())
	{
		PRESULTSET resultset = 0;
		char sql[2048];

		memset(sql, 0, sizeof(sql));
		sprintf(sql, "SELECT groupid,parentgroupid,groupname,grouptype FROM groupinfo_t WHERE account='%s'", accountInfo->getUserinfo()->getAccount().c_str());
		bodb_exec(sql, &resultset);
		if (resultset != 0)
		{
			for (int i=0; i<resultset->rscount; i++)
			{
				CFieldVariant varGroupId(resultset->rsvalues[i]->fieldvalues[0]);
				CFieldVariant varParentGroupId(resultset->rsvalues[i]->fieldvalues[1]);
				CFieldVariant varGroupName(resultset->rsvalues[i]->fieldvalues[2]);
				CFieldVariant varGroupType(resultset->rsvalues[i]->fieldvalues[3]);


				CGroupInfo::pointer groupInfo = CGroupInfo::create(varGroupId.getIntu(), varGroupName.getString(), (CGroupInfo::GroupType)varGroupType.getInt());

				// ??
				unsigned int parentGroupId = varParentGroupId.getIntu();
				if (parentGroupId > 0)
				{
					CGroupInfo::pointer parentGroupInfo;
					accountInfo->m_allgroups.find(parentGroupId, parentGroupInfo);
					groupInfo->parentgroup(parentGroupInfo);
				}

				accountInfo->m_allgroups.insert(groupInfo->groupid(), groupInfo);
			}

			bodb_free(resultset);
		}

		memset(sql, 0, sizeof(sql));
		sprintf(sql, "SELECT curgroupid,curdataid,curdatagroupid FROM accountsetting_t WHERE account='%s'", accountInfo->getUserinfo()->getAccount().c_str());
		bodb_exec(sql, &resultset);
		if (resultset != 0)
		{
			for (int i=0; i<resultset->rscount; i++)
			{
				CFieldVariant varCurGroupId(resultset->rsvalues[i]->fieldvalues[0]);
				CFieldVariant varCurDataId(resultset->rsvalues[i]->fieldvalues[1]);
				CFieldVariant varCurDataGroupId(resultset->rsvalues[i]->fieldvalues[2]);

				accountInfo->setcurgroupid(varCurGroupId.getIntu());
				accountInfo->setcurdataid(varCurDataId.getIntu());
				accountInfo->setcurdatagroupid(varCurDataGroupId.getIntu());
				break;
			}

			bodb_free(resultset);
		}

		// ??
		//CCompanyInfo::pointer companyInfo;
		//if (!m_companys.find(1, companyInfo))
		//{
		//	return;
		//}

		memset(sql, 0, sizeof(sql));
		sprintf(sql, "SELECT groupid FROM usercogroup_t WHERE account='%s'", accountInfo->getUserinfo()->getAccount().c_str());
		bodb_exec(sql, &resultset);
		if (resultset != 0)
		{
			for (int i=0; i<resultset->rscount; i++)
			{
				CFieldVariant varCoGroupId(resultset->rsvalues[i]->fieldvalues[0]);

				CCoGroupInfo::pointer cogroupInfo = CGC_POINTER_CAST<CCoGroupInfo>(gApplication->getAttribute(BMT_COGROUPS, varCoGroupId.getIntu()));
				if (cogroupInfo.get() != NULL)
				//if (this->m_cogroups.find(varCoGroupId.getIntu(), cogroupInfo))
				{
					accountInfo->m_cogroups.insert(cogroupInfo->groupid(), cogroupInfo);

					//if (companyInfo->hasCoGroup())
					//{
					//	companyInfo->m_userinfos.insert(cogroupInfo->groupid(), cogroupInfo);
					//}
				}
			}

			bodb_free(resultset);
		}

		// Load offevent
		if (accountInfo->getUserinfo()->m_offevents.empty())
		{
			memset(sql, 0, sizeof(sql));
			sprintf(sql, "SELECT id,event,fromtype,fromid,fromaccount,msgtype,newflag,message,msgsize,width,height,msgtime FROM offevents_t \
						 WHERE toaccount='%s'", accountInfo->getUserinfo()->getAccount().c_str());
			bodb_exec(sql, &resultset);
			if (resultset != 0)
			{
				for (int i=0; i<resultset->rscount; i++)
				{
					CFieldVariant varId(resultset->rsvalues[i]->fieldvalues[0]);
					CFieldVariant varEvent(resultset->rsvalues[i]->fieldvalues[1]);
					CFieldVariant varFromType(resultset->rsvalues[i]->fieldvalues[2]);
					CFieldVariant varFromId(resultset->rsvalues[i]->fieldvalues[3]);

					CFieldVariant varFromAccount(resultset->rsvalues[i]->fieldvalues[4]);
					CFieldVariant varMsgType(resultset->rsvalues[i]->fieldvalues[5]);
					CFieldVariant varNewFlag(resultset->rsvalues[i]->fieldvalues[6]);
					CFieldVariant varMessage(resultset->rsvalues[i]->fieldvalues[7]);
					CFieldVariant varMsgSize(resultset->rsvalues[i]->fieldvalues[8]);
					CFieldVariant varMsgTime(resultset->rsvalues[i]->fieldvalues[11]);

					CUserInfo::pointer fromUserInfo = CGC_POINTER_CAST<CUserInfo>(gApplication->getAttribute(BMT_ALLUSERS, varFromAccount.getString()));
					if (fromUserInfo.get() == NULL)
					{
						continue;
					}

					static long gMid = 0;

					int nEvent = varEvent.getInt();
					int msgtype = varMsgType.getInt();
					CMessageInfo::pointer messageInfo;
					switch (nEvent)
					{
					case 601:
					case 602:
						{
							CConversationInfo::ConversationType ct = (CConversationInfo::ConversationType)msgtype;
							messageInfo = CMessageInfo::create(++gMid, 0, ct, varNewFlag.getBool(true));
						}break;
					default:
						{
							switch (msgtype)
							{
							case 1:
							case 11:
								{
									std::string sString = varMessage.getString();
									messageInfo = CMessageInfo::create(++gMid, sString.size(), msgtype, varNewFlag.getBool(true));
									messageInfo->setdata(sString.c_str(), sString.size(), 0);
									if (msgtype == 11)
									{
										messageInfo->filesize(varMsgSize.getIntu());
									}

								}break;
							case 3:
								{
									std::string sFilename = varMessage.getString();
									messageInfo = CMessageInfo::create(++gMid, sFilename.size(), msgtype, varNewFlag.getBool(true));
									messageInfo->setdata(sFilename.c_str(), sFilename.size(), 0);
									CFieldVariant varWidth(resultset->rsvalues[i]->fieldvalues[9]);
									CFieldVariant varHeight(resultset->rsvalues[i]->fieldvalues[10]);

									messageInfo->imageWH(varWidth.getInt(), varHeight.getInt());
									messageInfo->filesize(varMsgSize.getIntu());

								}break;
							}
						}

					}
					if (messageInfo.get() == NULL)
					{
						continue;
					}

					int nFromType = varFromType.getInt();
					int nFromId = varFromId.getInt();

					CFromInfo::pointer fromInfo;
					CUserInfo::pointer toUserInfo;
					if (nFromType == (int)CFromInfo::FromUserInfo)
					{
						fromInfo = CFromInfo::create(fromUserInfo);
					}else if (nFromType == (int)CFromInfo::FromDialogInfo)
					{
						if (msgtype == CConversationInfo::CT_INVITE)
						{
							std::string sToAccount = varMessage.getString();
							toUserInfo = CGC_POINTER_CAST<CUserInfo>(gApplication->getAttribute(BMT_ALLUSERS, sToAccount));
							if (toUserInfo.get() == NULL)
							{
								continue;
							}
						}

						CDialogInfo::pointer dialogInfo = CGC_POINTER_CAST<CDialogInfo>(gApplication->getAttribute(BMT_DIALOGS, (ULONG)nFromId));
						if (dialogInfo.get() == NULL)
						{
							dialogInfo = CDialogInfo::create((long)nFromId, fromUserInfo);
							gApplication->setAttribute(BMT_DIALOGS, dialogInfo->dialogId(), dialogInfo);
							dialogInfo->m_members.insert(fromUserInfo->getAccount(), fromUserInfo);
							dialogInfo->m_members.insert(accountInfo->getUserinfo()->getAccount(), accountInfo->getUserinfo());
						}else
						{
							dialogInfo->m_members.insert(fromUserInfo->getAccount(), fromUserInfo);
						}
						fromInfo = CFromInfo::create(dialogInfo);
					}else
					{
						continue;
					}

					COfflineEvent::pointer offlineEvent = COfflineEvent::create(varEvent.getInt(), fromInfo, fromUserInfo, messageInfo);
					offlineEvent->toAccount(toUserInfo);
					offlineEvent->setId(varId.getIntu());
					offlineEvent->getMessage()->msgtime(varMsgTime.getIntu(0));
					accountInfo->getUserinfo()->m_offevents.add(offlineEvent);
				}

				bodb_free(resultset);
			}
		}else
		{
			memset(sql, 0, sizeof(sql));
			sprintf(sql, "DELETE FROM offevents_t WHERE toaccount='%s'", accountInfo->getUserinfo()->getAccount().c_str());
			bodb_exec(sql);
		}
	}
}

void string_replace(tstring & strBig, const tstring & strsrc, const tstring &strdst)
{
	tstring::size_type pos=0;
	tstring::size_type srclen=strsrc.size();
	tstring::size_type dstlen=strdst.size();

	while ((pos=strBig.find(strsrc, pos)) != tstring::npos)
	{
		strBig.replace(pos, srclen, strdst);
		pos += dstlen;
	}
}

void CAVSProxy::addFriendGroup(CAccountInfo::pointer accountInfo, CGroupInfo::pointer groupInfo)
{
	BOOST_ASSERT (accountInfo.get() != 0);
	BOOST_ASSERT (accountInfo->getUserinfo().get() != 0);
	BOOST_ASSERT (groupInfo.get() != 0);

	accountInfo->m_allgroups.insert(groupInfo->groupid(), groupInfo);

	if (bodb_isopened())
	{
		std::string groupname = groupInfo->name();
		string_replace(groupname, "'", "''");

		char sql[2048];
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "INSERT INTO groupinfo_t (account,groupid,parentgroupid,groupname,grouptype)\
					 VALUES('%s',%ld, %d,'%s',%d)",
			accountInfo->getUserinfo()->getAccount().c_str(), groupInfo->groupid(),
			groupInfo->parentgroupid(), groupname.c_str(),
			(int)groupInfo->type());
		bodb_exec(sql);
	}
}

void CAVSProxy::addFriendinfo(CAccountInfo::pointer accountInfo, CFriendInfo::pointer friendInfo)
{
	BOOST_ASSERT (accountInfo.get() != 0);
	BOOST_ASSERT (friendInfo.get() != 0);
	BOOST_ASSERT (friendInfo->userinfo().get() != 0);

	accountInfo->m_allfriends.insert(friendInfo->userinfo()->getAccount(), friendInfo);

	if (bodb_isopened())
	{
		char sql[2048];
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "INSERT INTO friendinfo_t (account,groupid,substate,note)\
					 VALUES('%s',%d,%d,'%s')",
			friendInfo->userinfo()->getAccount().c_str(), friendInfo->groupid(),
			friendInfo->substate(), friendInfo->note().c_str());
		bodb_exec(sql);
	}
}

void CAVSProxy::updateGroupName(CAccountInfo::pointer accountInfo, CGroupInfo::pointer groupInfo, const tstring & newName)
{
	BOOST_ASSERT (accountInfo.get() != 0);
	BOOST_ASSERT (groupInfo.get() != 0);
	groupInfo->name(newName);

	if (bodb_isopened())
	{

	}

	//if (m_dbservice.isopendb())
	{
		/*
		CTableInfo::pointer tableInfo = m_dbservice.getTableInfo(_T("groupinfo_t"));

		CFieldInfo::pointer fieldInfo = tableInfo->getFieldInfo(_T("account"));
		CFieldVariant::pointer variant = CFieldVariant::create(fieldInfo->type());
		variant->setString(accountInfo->getUserinfo()->getAccount());
		std::list<CFieldCompare::pointer> wheres;
		wheres.push_back(CFieldCompare::create(CFieldCompare::FCT_EQUAL, fieldInfo, variant));

		fieldInfo = tableInfo->getFieldInfo(_T("groupid"));
		variant = CFieldVariant::create(fieldInfo->type());
		variant->setIntu(groupInfo->groupid());
		wheres.push_back(CFieldCompare::create(CFieldCompare::FCT_EQUAL, fieldInfo, variant));

		CRecordLine::pointer recordLine = CRecordLine::create(tableInfo);
		recordLine->addVariant(_T("groupname"), groupInfo->name());

		std::list<std::list<CFieldCompare::pointer>> topwheres;
		topwheres.push_back(wheres);
		m_dbservice.update(tableInfo, topwheres, recordLine);
		*/
	}
}

void CAVSProxy::deleteGroup(CAccountInfo::pointer accountInfo, unsigned int groupId)
{
	BOOST_ASSERT (accountInfo.get() != 0);
	accountInfo->m_allgroups.remove(groupId);

	if (bodb_isopened())
	{
		char sql[2048];
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "DELETE FROM groupinfo_t WHERE account='%s' AND groupid=%d", accountInfo->getUserinfo()->getAccount().c_str(), groupId);
		bodb_exec(sql);
	}
}

long CAVSProxy::getNextDialogId(void)
{
	++m_currentDialogId;
	updateSystemSetting();
	return m_currentDialogId;
}

void CAVSProxy::loadSystemSetting(void)
{
	PRESULTSET resultset = 0;
	bodb_exec("SELECT curdialogid FROM systemsetting_t", &resultset);

	if (resultset != 0)
	{
		for (int i=0; i<resultset->rscount; i++)
		{
			CFieldVariant varDialogId(resultset->rsvalues[i]->fieldvalues[0]);

			this->m_currentDialogId = (long)varDialogId.getInt();
			break;
		}

		bodb_free(resultset);
		resultset = 0;
	}

}

void CAVSProxy::updateSystemSetting(void)
{
	if (bodb_isopened())
	{
		char sql[1024];
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "UPDATE systemsetting_t SET curdialogid=%ld", m_currentDialogId);
		bodb_exec(sql);
	}
}

void CAVSProxy::updateAccountSetting(CAccountInfo::pointer accountInfo)
{
	BOOST_ASSERT (accountInfo.get() != 0);
	BOOST_ASSERT (accountInfo->getUserinfo().get() != 0);

	if (bodb_isopened())
	{
		char sql[2048];
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "SELECT * FROM accountsetting_t WHERE account='%s'", accountInfo->getUserinfo()->getAccount().c_str());
		int ret = bodb_exec(sql);

		if (ret < 1)
		{
			memset(sql, 0, sizeof(sql));
			sprintf(sql, "INSERT INTO accountsetting_t(account,curgroupid,curdataid,curdatagroupid) \
						 VALUES('%s',%ld,%ld,%ld)",
						 accountInfo->getUserinfo()->getAccount().c_str(), accountInfo->getcurgroupid(),
						 accountInfo->getcurdataid(), accountInfo->getcurdatagroupid());
		}else
		{
			memset(sql, 0, sizeof(sql));
			sprintf(sql, "UPDATE accountsetting_t SET curgroupid=%ld,curdataid=%ld,curdatagroupid=%ld WHERE account='%s'",
				accountInfo->getcurgroupid(),accountInfo->getcurdataid(),
				accountInfo->getcurdatagroupid(), accountInfo->getUserinfo()->getAccount().c_str());
		}
		bodb_exec(sql);
	}
}

unsigned int CAVSProxy::addCompany(const std::string & coName)
{

	return 0;
}

bool CAVSProxy::load(void)
{
	if (!bodb_init(m_path.c_str()))
	{
		return false;
	}

	std::string sql("USE ");
	sql.append(biwoo_db_name);
	if (bodb_exec(sql.c_str()) != 0)
	{
		return false;
	}

	PRESULTSET resultset = 0;
	bodb_exec("SELECT * FROM userinfo_t", &resultset);

	if (resultset != 0)
	{
		for (int i=0; i<resultset->rscount; i++)
		{
			CFieldVariant varAccount(resultset->rsvalues[i]->fieldvalues[0]);
			CFieldVariant varPassword(resultset->rsvalues[i]->fieldvalues[1]);
			CFieldVariant varUserName(resultset->rsvalues[i]->fieldvalues[2]);
			CFieldVariant varNick(resultset->rsvalues[i]->fieldvalues[3]);
			CFieldVariant varGender(resultset->rsvalues[i]->fieldvalues[4]);
			CFieldVariant varBirthday(resultset->rsvalues[i]->fieldvalues[5]);
			CFieldVariant varPhone(resultset->rsvalues[i]->fieldvalues[6]);
			CFieldVariant varEmail(resultset->rsvalues[i]->fieldvalues[7]);

			CUserInfo::pointer userInfo = CUserInfo::create(varAccount.getString(), varPassword.getString());
			userInfo->setUserName(varUserName.getString());
			userInfo->setNick(varNick.getString());
			userInfo->setGender(varGender.getInt());
			userInfo->setBirthday(varBirthday.getString());
			userInfo->setPhone(varPhone.getString());
			userInfo->setEmail(varEmail.getString());

			gApplication->setAttribute(BMT_ALLUSERS, userInfo->getAccount(), userInfo);
		}

		bodb_free(resultset);
		resultset = 0;
	}

	bodb_exec("SELECT coid,name FROM company_t", &resultset);
	if (resultset != 0)
	{
		for (int i=0; i<resultset->rscount; i++)
		{
			CFieldVariant varCompanyId(resultset->rsvalues[i]->fieldvalues[0]);
			CFieldVariant varCompanyName(resultset->rsvalues[i]->fieldvalues[1]);
			CCompanyInfo::pointer companyInfo = CCompanyInfo::create(varCompanyId.getIntu(), varCompanyName.getString());
			gApplication->setAttribute(BMT_COMPANYS, companyInfo->id(), companyInfo);
			//m_companys.insert(companyInfo->id(), companyInfo);
		}

		bodb_free(resultset);
		resultset = 0;
	}

	// ??
	CCompanyInfo::pointer companyInfo = CGC_POINTER_CAST<CCompanyInfo>(gApplication->getAttribute(BMT_COMPANYS, 1));
	if (companyInfo.get() == NULL)
	//if (!m_companys.find(1, companyInfo))
	{
		return false;
	}

	// ??
	// cogroup_t
	bodb_exec("SELECT groupid,parentgroupid,name,type FROM cogroup_t WHERE coid=1", &resultset);
	if (resultset != 0)
	{
		for (int i=0; i<resultset->rscount; i++)
		{
			CFieldVariant varGroupId(resultset->rsvalues[i]->fieldvalues[0]);
			CFieldVariant varParentGroupId(resultset->rsvalues[i]->fieldvalues[1]);
			CFieldVariant varGroupName(resultset->rsvalues[i]->fieldvalues[2]);
			CFieldVariant varGroupType(resultset->rsvalues[i]->fieldvalues[3]);

			CCoGroupInfo::pointer groupInfo = CCoGroupInfo::create(companyInfo, varGroupId.getIntu(), varGroupName.getString(), (CCoGroupInfo::GroupType)varGroupType.getInt());

			// ??
			unsigned int parentGroupId = varParentGroupId.getIntu();
			if (parentGroupId > 0)
			{
				CCoGroupInfo::pointer parentGroupInfo;
				companyInfo->m_allgroups.find(parentGroupId, parentGroupInfo);
				groupInfo->parentgroup(parentGroupInfo);
			}

			companyInfo->m_allgroups.insert(groupInfo->groupid(), groupInfo);
			gApplication->setAttribute(BMT_COGROUPS, groupInfo->groupid(), groupInfo);
			//m_cogroups.insert(groupInfo->groupid(), groupInfo);
		}

		bodb_free(resultset);
		resultset = 0;
	}

	// usercogroup_t
	bodb_exec("SELECT account,groupid FROM usercogroup_t", &resultset);
	if (resultset != 0)
	{
		for (int i=0; i<resultset->rscount; i++)
		{
			CFieldVariant varAccount(resultset->rsvalues[i]->fieldvalues[0]);
			CFieldVariant varGroupId(resultset->rsvalues[i]->fieldvalues[1]);

			CUserInfo::pointer userInfo = CGC_POINTER_CAST<CUserInfo>(gApplication->getAttribute(BMT_ALLUSERS, varAccount.getString()));
			//if (!this->m_users.find(varAccount.getString(), userInfo))
			if (userInfo.get() == NULL)
			{
				continue;
			}

			if (!companyInfo->m_userinfos.exist(userInfo->getAccount()))
			{
				companyInfo->m_userinfos.insert(userInfo->getAccount(), userInfo);
			}

			CCoGroupInfo::pointer cogroupInfo;
			if (companyInfo->m_allgroups.find(varGroupId.getIntu(), cogroupInfo))
			{
				userInfo->m_companys.insert(companyInfo->id(), companyInfo);
				userInfo->m_cogroups.insert(cogroupInfo->groupid(), cogroupInfo);
				cogroupInfo->m_userinfos.insert(userInfo->getAccount(), userInfo);
			}
		}

		bodb_free(resultset);
		resultset = 0;
	}

	loadSystemSetting();

	return true;
}

CAVSProxy::CAVSProxy(void)
: m_path("")
, m_currentDialogId(0)

{

}

CAVSProxy::~CAVSProxy(void)
{
	close();
}


void CAVSProxy::close(void)
{
	bodb_exit();
	m_rejects.clear();
	//m_commfMgr.clearAll();
}
