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

// groupinfo.h file here
#ifndef __groupinfo_h__
#define __groupinfo_h__

#include <stl/stldef.h>
#include <boost/shared_ptr.hpp>

class CGroupInfo
{
public:
	enum GroupType
	{
		GT_Normal = 1
		, GT_Group
	};

	typedef boost::shared_ptr<CGroupInfo> pointer;
	static CGroupInfo::pointer create(unsigned long groupid, const tstring & name, GroupType type)
	{
		return CGroupInfo::pointer(new CGroupInfo(groupid, name, type));
	}
	static CGroupInfo::pointer create(const tstring & name, GroupType type)
	{
		return CGroupInfo::pointer(new CGroupInfo(name, type));
	}
	CGroupInfo(const CGroupInfo & v)
	{
		equal(v);
	}
	const CGroupInfo & operator = (const CGroupInfo & v)
	{
		equal(v);
		return *this;
	}

	unsigned long groupid(void) const {return m_groupid;}

	void parentgroup(CGroupInfo::pointer newv) {m_parentgroup = newv;}
	CGroupInfo::pointer parentgroup(void) const {return m_parentgroup;}
	unsigned long parentgroupid(void) const {return m_parentgroup.get() == 0 ? 0 : m_parentgroup->groupid();}

	void name(const tstring & newv) {m_name = newv;}
	const tstring & name(void) const {return m_name;}
	GroupType type(void) const {return m_type;}

protected:
	void equal(const CGroupInfo & v)
	{
		this->m_name = v.name();
		this->m_type = v.type();
		this->m_parentgroup = v.parentgroup();
	}
public:
	CGroupInfo(unsigned long groupid, const tstring & name, GroupType type)
		: m_groupid(groupid), m_name(name), m_type(type)
	{}
	CGroupInfo(const tstring & name, GroupType type)
		: m_groupid(0), m_name(name), m_type(type)
	{}
	~CGroupInfo(void)
	{
	}

private:
	unsigned long m_groupid;
	CGroupInfo::pointer m_parentgroup;
	tstring m_name;
	GroupType m_type;
};


#endif // __groupinfo_h__

