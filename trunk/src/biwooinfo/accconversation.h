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

// accconversation.h file here
#ifndef __accconversation_h__
#define __accconversation_h__

#include <ThirdParty/stl/stldef.h>
#include <ThirdParty/stl/locklist.h>
#include <boost/shared_ptr.hpp>
#include "conversationinfo.h"
#include "frominfo.h"
//#include "dialoginfo.h"
//#include "userinfo.h"

class CAccountConversation
{
public:
	typedef boost::shared_ptr<CAccountConversation> pointer;

	static CAccountConversation::pointer create(CFromInfo::pointer fromInfo)
	{
		return CAccountConversation::pointer(new CAccountConversation(fromInfo));
	}

	//static CAccountConversation::pointer create(CUserInfo::pointer fromAccount)
	//{
	//	return CAccountConversation::pointer(new CAccountConversation(fromAccount));
	//}
	//static CAccountConversation::pointer create(CDialogInfo::pointer fromDialog)
	//{
	//	return CAccountConversation::pointer(new CAccountConversation(fromDialog));
	//}

	CLockList<CConversationInfo::pointer>	m_conversations;
	//CLockList<CConversationInfo::pointer>	m_unreads;

	CFromInfo::pointer fromInfo(void) const {return m_fromInfo;}

	//CUserInfo::pointer fromAccount(void) const {return m_fromAccount;}
	//CDialogInfo::pointer fromDialog(void) const {return m_fromDialog;}

	CAccountConversation(CFromInfo::pointer fromInfo)
		: m_fromInfo(fromInfo)
	{
		BOOST_ASSERT (m_fromInfo.get() != 0);
	}

	//CAccountConversation(CUserInfo::pointer fromAccount)
	//	: m_fromAccount(fromAccount)
	//{
	//	BOOST_ASSERT (m_fromAccount.get() != 0);
	//}
	//CAccountConversation(CDialogInfo::pointer fromDialog)
	//	: m_fromDialog(fromDialog)
	//{
	//	BOOST_ASSERT (m_fromDialog.get() != 0);
	//}
	~CAccountConversation(void)
	{
		m_conversations.clear();
		//m_unreads.clear();
	}

private:
	//CDialogInfo::pointer m_fromDialog;
	//CUserInfo::pointer	m_fromAccount;
	CFromInfo::pointer m_fromInfo;
};


#endif // __accconversation_h__