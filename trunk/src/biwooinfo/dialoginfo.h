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

// dialoginfo.h file here
#ifndef __dialoginfo_h__
#define __dialoginfo_h__

#include <stl/stldef.h>
#include <stl/lockmap.h>
#include <boost/shared_ptr.hpp>
#include <CGCBase/cgcpointer.h>
//#include "userinfo.h"

class CUserInfo;
typedef boost::shared_ptr<CUserInfo> CUserInfoPointer;

class CDialogInfo
	: public cgc::cgcPointer
{
public:
	typedef boost::shared_ptr<CDialogInfo> pointer;
	static CDialogInfo::pointer create(long did, CUserInfoPointer manager)
	{
		return CDialogInfo::pointer(new CDialogInfo(did, manager));
	}

	CLockMap<std::string, CUserInfoPointer> m_members;

	bool empty(void) const {return m_members.empty();}
	long dialogId(void) const {return m_dialogId;}
	void manager(CUserInfoPointer newv) {m_manager = newv;}
	CUserInfoPointer manager(void) const {return m_manager;}

	void history(bool newv) {m_history = newv;}
	bool history(void) const {return m_history;}

	CDialogInfo(long did, CUserInfoPointer manager)
		: m_dialogId(did), m_manager(manager)
		, m_history(false)
	{
		BOOST_ASSERT (m_manager.get() != 0);
	}
	~CDialogInfo(void)
	{
		m_members.clear();
	}

private:
	long					m_dialogId;
	CUserInfoPointer		m_manager;
	bool					m_history;
};


#endif // __dialoginfo_h__
