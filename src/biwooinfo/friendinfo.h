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

// friendinfo.h file here
#ifndef __friendinfo_h__
#define __friendinfo_h__

#include <ThirdParty/stl/stldef.h>
#include <boost/shared_ptr.hpp>
#include "userinfo.h"

class CFriendInfo
{
public:
	typedef boost::shared_ptr<CFriendInfo> pointer;
	static CFriendInfo::pointer create(CUserInfo::pointer friendui)
	{
		return CFriendInfo::pointer(new CFriendInfo(friendui));
	}

	CUserInfo::pointer userinfo(void) const {return m_friend;}
	void substate(short newv) {m_substate = newv;}
	short substate(void) const {return m_substate;}
	void note(const tstring & newv) {m_note = newv;}
	const tstring & note(void) const {return m_note;}

	void parentgroup(CGroupInfo::pointer newv) {m_parentgroup = newv;}
	CGroupInfo::pointer parentgroup(void) const {return m_parentgroup;}
	unsigned int groupid(void) const {return m_parentgroup.get() == 0 ? 0 : m_parentgroup->groupid();}

	CFriendInfo(CUserInfo::pointer friendui)
		: m_friend(friendui)
		, m_substate(0), m_note(_T(""))
	{
		BOOST_ASSERT (m_friend.get() != 0);
	}
	~CFriendInfo(void)
	{
	}

private:
	CUserInfo::pointer	m_friend;
	CGroupInfo::pointer m_parentgroup;
	short				m_substate;	// subscript state
	tstring				m_note;

};


#endif // __friendinfo_h__

