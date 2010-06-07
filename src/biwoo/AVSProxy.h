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

// AVSProxy.h file here
#ifndef __AVSProxy_h__
#define __AVSProxy_h__

#include <boost/shared_ptr.hpp>
#include <CGCBase/includeapp.h>
#include <CGCServices/Bodb/cgcBodb.h>
#include <ThirdParty/stl/lockmap.h>
#include <fstream>
#include "../biwooinfo/accountinfo.h"
#include "../biwooinfo/companyinfo.h"
#include "../biwooinfo/dialoginfo.h"
#include "../biwooinfo/conversationinfo.h"

//#include "CommConferMgr.h"
//#include "avconfer/AVConference.h"

enum BiwooMapType
{
	BMT_ALLUSERS	= 100	// account(tstring)		> CUserInfo::pointer
	, BMT_ACCOUNTIDS		// accountid(tstring)	> CAccountInfo::pointer
	, BMT_ACCOUNTS			// account(tstring)		> CAccountInfo::pointer
	, BMT_COMPANYS			// companyId(ULONG)		> CCompanyInfo::pointer
	, BMT_COGROUPS			// cogroupId(ULONG)		> CCoGroupInfo::pointer
	, BMT_OFFLINEMSG		// id(ULONG)			> CMessageInfo::pointer
	, BMT_DIALOGS			// dialogId(ULONG)		> CDialogInfo::pointer
};

class CAVSProxy
{
public:
	typedef boost::shared_ptr<CAVSProxy> pointer;
	static CAVSProxy::pointer create(cgcBodb::pointer bodbService, CBodbHandler::pointer bodbHandler)
	{
		return CAVSProxy::pointer(new CAVSProxy(bodbService, bodbHandler));
	}

	CLockMap<long, bool> m_rejects;

	//CCommConferMgr	m_commfMgr;
	//CAVConference	m_conference;

public:
	void deleteDbOffEvent(unsigned long id);
	void addOffEvent(CUserInfo::pointer userInfo, COfflineEvent::pointer offlineEvent);
	void addUserinfo(CUserInfo::pointer userInfo);
	void updateUserinfo(CUserInfo::pointer userInfo);
	void deleteUserinfo(const tstring & sAccount);
	void updatePassword(const tstring & sAccount, const tstring & sNewPassword);

	void loadAccountInfo(CAccountInfo::pointer accountInfo);
	void addFriendGroup(CAccountInfo::pointer accountInfo, CGroupInfo::pointer groupInfo);
	void addFriendinfo(CAccountInfo::pointer accountInfo, CFriendInfo::pointer friendInfo);
	void updateGroupName(CAccountInfo::pointer accountInfo, CGroupInfo::pointer groupInfo, const tstring & newName);
	void deleteGroup(CAccountInfo::pointer accountInfo, unsigned int groupId);

	void loadSystemSetting(void);
	void updateSystemSetting(void);
	void updateAccountSetting(CAccountInfo::pointer accountInfo);
	unsigned int addCompany(const std::string & coName);
	
	bool load(const tstring & databaseName);
	void close(void);

	long getNextDialogId(void);

public:
	CAVSProxy(cgcBodb::pointer bodbServic, CBodbHandler::pointer bodbHandler);
	~CAVSProxy(void);

private:
	cgcBodb::pointer m_bodbService;
	CBodbHandler::pointer m_bodbHandler;
	long m_currentDialogId;
};


extern CAVSProxy::pointer gAVSProxy;

#endif // __AVSProxy_h__
