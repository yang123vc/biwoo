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

// conversationinfo.h file here
#ifndef __conversationinfo_h__
#define __conversationinfo_h__

#include <ThirdParty/stl/stldef.h>
#include <boost/shared_ptr.hpp>
#include "messageinfo.h"
#include "userinfo.h"
#include "frominfo.h"

class CConversationInfo
{
public:
	enum ConversationType
	{
		CT_HISTORY	= 0x01
		, CT_MSG	= 0x10
		, CT_CREATE	= 0x20
		, CT_INVITE
		, CT_QUIT
		, CT_ACCEPT	= 0x30
		, CT_CANCEL
		, CT_REJECT
		, CT_SUCCEED

	};

	typedef boost::shared_ptr<CConversationInfo> pointer;
	static CConversationInfo::pointer create(CFromInfo::pointer fromInfo, CUserInfo::pointer sayAccount, CMessageInfo::pointer message, ConversationType ct = CT_MSG)
	{
		return CConversationInfo::pointer(new CConversationInfo(fromInfo, sayAccount, message, ct));
	}

	ConversationType conversationType(void)const {return m_conversationType;}
	CFromInfo::pointer fromInfo(void) const {return m_fromInfo;}
	CUserInfo::pointer sayAccount(void) const {return m_sayAccount;}
	CMessageInfo::pointer message(void) const {return m_message;}

	void fromAccount(CUserInfo::pointer newv) {m_fromAccount = newv;}
	CUserInfo::pointer fromAccount(void) const {return m_fromAccount;}

	void readState(bool newv) {m_readState = newv;}
	bool readState(void) const {return m_readState;}

	void percent(float newv) {m_percent = newv;}
	float percent(void) const {return m_percent;}

	CConversationInfo(CFromInfo::pointer fromInfo, CUserInfo::pointer sayAccount, CMessageInfo::pointer message, ConversationType ct)
		: m_conversationType(ct), m_fromInfo(fromInfo), m_sayAccount(sayAccount), m_message(message)
		, m_readState(true), m_percent(0.0)
	{
		BOOST_ASSERT (m_fromInfo.get() != 0);
		BOOST_ASSERT (m_sayAccount.get() != 0);
		BOOST_ASSERT (m_message.get() != 0);
	}
	~CConversationInfo(void)
	{
	}

private:
	ConversationType		m_conversationType;
	CFromInfo::pointer		m_fromInfo;
	CUserInfo::pointer		m_fromAccount;
	CUserInfo::pointer		m_sayAccount;
	CMessageInfo::pointer	m_message;
	bool					m_readState;		// default true
	float					m_percent;
};


#endif // __conversationinfo_h__

