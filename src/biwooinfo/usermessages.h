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

// usermessages.h file here
#ifndef __usermessages_h__
#define __usermessages_h__

#include <ThirdParty/stl/stldef.h>
#include <ThirdParty/stl/lockmap.h>
#include <boost/shared_ptr.hpp>
#include "userinfo.h"
#include "messageinfo.h"

class CUserMessages
{
public:
	typedef boost::shared_ptr<CUserMessages> pointer;
	static CUserMessages::pointer create(CUserInfo::pointer userinfo)
	{
		return CUserMessages::pointer(new CUserMessages(userinfo));
	}

	CUserMessages(CUserInfo::pointer userinfo)
		: m_userinfo(userinfo)
	{
		BOOST_ASSERT (m_userinfo.get() != NULL);
	}
	~CUserMessages(void)
	{
		m_messages.clear();
	}

	CLockMap<long, CMessageInfo::pointer> m_messages;

public:
	CUserInfo::pointer userinfo(void) const {return m_userinfo;}

private:
	CUserInfo::pointer m_userinfo;

};


#endif // __usermessages_h__