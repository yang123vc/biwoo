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

// offlineevent.h file here
#ifndef __offlineevent_h__
#define __offlineevent_h__

#include <stl/stldef.h>
#include <boost/shared_ptr.hpp>
#include "messageinfo.h"
#include "frominfo.h"

const CMessageInfo::pointer MessageInfoEmpty;
//class CUserInfo;
//typedef boost::shared_ptr<CUserInfo> CUserInfoPointer;

class COfflineEvent
{
public:
	typedef boost::shared_ptr<COfflineEvent> pointer;
	static COfflineEvent::pointer create(int offevent, CFromInfo::pointer fromInfo, CUserInfoPointer fromAccount, CMessageInfo::pointer messageInfo = MessageInfoEmpty)
	{
		return COfflineEvent::pointer(new COfflineEvent(offevent, fromInfo, fromAccount, messageInfo));
	}

	COfflineEvent(int offevent, CFromInfo::pointer fromInfo, CUserInfoPointer fromAccount, CMessageInfo::pointer messageInfo)
		: m_id(0), m_event(offevent)
		, m_fromInfo(fromInfo), m_fromAccount(fromAccount)
		, m_message(messageInfo)

	{
		BOOST_ASSERT (m_fromInfo.get() != NULL);
		BOOST_ASSERT (m_fromAccount.get() != NULL);
	}
	~COfflineEvent(void)
	{}

public:
	int getEvent(void) const {return m_event;}

	CFromInfo::pointer fromInfo(void) const {return m_fromInfo;}
	CUserInfoPointer getFromAccount(void) const {return m_fromAccount;}
	CMessageInfo::pointer getMessage(void) const {return m_message;}

	void setId(unsigned long newv) {m_id = newv;}
	unsigned long getId(void) const {return m_id;}

	void toAccount(CUserInfoPointer newv ) {m_toAccount = newv;}
	CUserInfoPointer toAccount(void) const {return m_toAccount;}

private:
	unsigned long m_id;
	int m_event;
	CFromInfo::pointer m_fromInfo;
	CUserInfoPointer m_fromAccount;
	CUserInfoPointer m_toAccount;
	CMessageInfo::pointer m_message;

};

#endif // __offlineevent_h__
