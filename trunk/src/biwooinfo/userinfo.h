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

// userinfo.h file here
#ifndef __userinfo_h__
#define __userinfo_h__

#include <stl/stldef.h>
#include <stl/locklist.h>
#include <boost/shared_ptr.hpp>
#include <CGCBase/cgcpointer.h>
using namespace cgc;
#include "offlineevent.h"
//#include "accconversation.h"

class CCompanyInfo;
typedef boost::shared_ptr<CCompanyInfo> CCompanyInfoPointer;

class CCoGroupInfo;
typedef boost::shared_ptr<CCoGroupInfo> CCoGroupInfoPointer;

class CUserInfo
	: public cgc::cgcPointer
{
public:
	enum UserLineState
	{
		UserOffLineState
		, UserOnLineState
		, UserAwayState
	};

	typedef boost::shared_ptr<CUserInfo> pointer;
	static CUserInfo::pointer create(const tstring & account, const tstring & password)
	{
		return CUserInfo::pointer(new CUserInfo(account, password));
	}

	CUserInfo(const tstring & account, const tstring & password)
		: m_sAccount(account), m_sPassword(password)
		, m_sUserName(_T("")), m_sNick(_T(""))
		, m_gender(0), m_birthday(_T("")), m_phone(_T("")), m_email(_T(""))
		//, m_sAccountId(_T("")), m_sSessionId(_T(""))
		, m_lineState(UserOffLineState)
	{}
	~CUserInfo(void)
	{
		m_companys.clear();
		m_cogroups.clear();
		m_offevents.clear();
		//m_conversations.clear();
		//m_friends.clear();
	}

	CLockMap<unsigned int, CCompanyInfoPointer> m_companys;
	CLockMap<unsigned int, CCoGroupInfoPointer> m_cogroups;
	CLockList<COfflineEvent::pointer> m_offevents;
	//CLockList<CAccountConversation::pointer> m_conversations;
	//CLockMap<tstring, CUserInfo::pointer> m_friends;

public:
	void setAccount(const tstring & newValue) {m_sAccount = newValue;}
	const tstring & getAccount(void) const {return m_sAccount;}

	void setPassword(const tstring & newValue) {m_sPassword = newValue;}
	const tstring & getPassword(void) const {return m_sPassword;}

	void setUserName(const tstring  newv) {m_sUserName = newv;}
	const tstring & getUserName(void) const {return m_sUserName;}

	void setNick(const tstring & newv) {m_sNick = newv;}
	const tstring & getNick(void) const {return m_sNick;}

	void setGender(short newv) {m_gender = newv;}
	short getGender(void) const {return m_gender;}
	void setBirthday(const tstring & newv) {m_birthday = newv;}
	const tstring & getBirthday(void) const {return m_birthday;}
	void setPhone(const tstring & newv) {m_phone = newv;}
	const tstring & getPhone(void) const {return m_phone;}
	void setEmail(const tstring & newv) {m_email = newv;}
	const tstring & getEmail(void) const {return m_email;}

	//void setAccountId(const tstring & newv) {m_sAccountId = newv;}
	//const tstring & getAccountId(void) const {return m_sAccountId;}
	//void setSessionId(const tstring & newValue) {m_sSessionId = newValue;}
	//const tstring & getSessionId(void) const {return m_sSessionId;}

	void setLineState(UserLineState newv) {m_lineState = newv;}
	UserLineState getLineState(void) const {return m_lineState;}
	bool isOfflineState(void) const {return m_lineState == UserOffLineState;}

private:
	tstring m_sAccount;
	tstring m_sPassword;
	tstring m_sUserName;
	tstring m_sNick;
	short	m_gender;		// 1: male, 2: female, default 0
	tstring m_birthday;
	tstring m_phone;
	tstring m_email;

	UserLineState	m_lineState;
	//tstring m_sAccountId;	// Account@DOMAIN
	//tstring m_sSessionId;	// For CGC
};


#endif // __userinfo_h__

