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

// datagroup.h file here
#ifndef __datagroup_h__
#define __datagroup_h__

#include <ThirdParty/stl/stldef.h>
#include <boost/shared_ptr.hpp>
#include "userinfo.h"

class CDataGroup
{
public:
	typedef boost::shared_ptr<CDataGroup> pointer;
	static CDataGroup::pointer create(long groupid, const tstring & name, CUserInfo::pointer owner)
	{
		return CDataGroup::pointer(new CDataGroup(groupid, name, owner));
	}

	CDataGroup(const CDataGroup & v)
	{
		equal(v);
	}
	const CDataGroup & operator = (const CDataGroup & v)
	{
		equal(v);
		return *this;
	}

	long groupid(void) const {return m_groupid;}

	void name(const tstring & newv) {m_name = newv;}
	const tstring & name(void) const {return m_name;}

	CUserInfo::pointer owner(void) const {return m_owner;}

	void parentgroup(CDataGroup::pointer newv) {m_parentgroup = newv;}
	CDataGroup::pointer parentgroup(void) const {return m_parentgroup;}


protected:
	void equal(const CDataGroup & v)
	{
		this->m_name = v.name();
		this->m_owner = v.owner();
		this->m_parentgroup = v.parentgroup();
	}
public:
	CDataGroup(long groupid, const tstring & name, CUserInfo::pointer owner)
		: m_groupid(groupid), m_name(name), m_owner(owner)
	{
		BOOST_ASSERT (m_owner.get() != 0);
	}
	~CDataGroup(void)
	{
	}

private:
	long m_groupid;
	tstring m_name;
	CUserInfo::pointer m_owner;
	CDataGroup::pointer m_parentgroup;
};


#endif // __datagroup_h__

