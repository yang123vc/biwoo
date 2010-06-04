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

// cogroupinfo.h file here
#ifndef __cogroupinfo_h__
#define __cogroupinfo_h__

#include <ThirdParty/stl/stldef.h>
#include <ThirdParty/stl/locklist.h>
#include <boost/shared_ptr.hpp>
#include <CGCBase/cgcpointer.h>
//#include "userinfo.h"

class CCompanyInfo;
typedef boost::shared_ptr<CCompanyInfo> CCompanyInfoPointer;

class CUserInfo;
typedef boost::shared_ptr<CUserInfo> CUserInfoPointer;

class CCoGroupInfo
	: public cgc::cgcPointer
{
public:
	enum GroupType
	{
		GT_Normal = 1
		, GT_Group
	};

	typedef boost::shared_ptr<CCoGroupInfo> pointer;
	static CCoGroupInfo::pointer create(CCompanyInfoPointer companyInfo, unsigned long groupid, const tstring & name, GroupType type)
	{
		return CCoGroupInfo::pointer(new CCoGroupInfo(companyInfo, groupid, name, type));
	}
	static CCoGroupInfo::pointer create(CCompanyInfoPointer companyInfo, const tstring & name, GroupType type)
	{
		return CCoGroupInfo::pointer(new CCoGroupInfo(companyInfo, name, type));
	}
	CCoGroupInfo(const CCoGroupInfo & v)
	{
		equal(v);
	}
	const CCoGroupInfo & operator = (const CCoGroupInfo & v)
	{
		equal(v);
		return *this;
	}

	CLockMap<std::string, CUserInfoPointer>	m_userinfos;

	CCompanyInfoPointer companyInfo(void) const {return m_companyInfo;}
	unsigned long groupid(void) const {return m_groupid;}

	void parentgroup(CCoGroupInfo::pointer newv) {m_parentgroup = newv;}
	CCoGroupInfo::pointer parentgroup(void) const {return m_parentgroup;}
	unsigned long parentgroupid(void) const {return m_parentgroup.get() == 0 ? 0 : m_parentgroup->groupid();}

	void name(const tstring & newv) {m_name = newv;}
	const tstring & name(void) const {return m_name;}
	GroupType type(void) const {return m_type;}

	int onlineCount(void) const {return m_onlineCount;}
	int iOnlineCount(bool bCountParent = true)
	{
		if (bCountParent && m_parentgroup.get() != NULL)
		{
			m_parentgroup->iOnlineCount(bCountParent);
		}
		return ++m_onlineCount;
	}
	int dOnlineCount(bool bCountParent = true)
	{
		if (bCountParent && m_parentgroup.get() != NULL)
		{
			m_parentgroup->dOnlineCount(bCountParent);
		}
		return --m_onlineCount;
	}

protected:
	void equal(const CCoGroupInfo & v)
	{
		this->m_name = v.name();
		this->m_type = v.type();
		this->m_parentgroup = v.parentgroup();
	}
public:
	CCoGroupInfo(CCompanyInfoPointer companyInfo, unsigned long groupid, const tstring & name, GroupType type)
		: m_companyInfo(companyInfo), m_groupid(groupid), m_name(name), m_type(type)
		, m_onlineCount(0)
	{
		BOOST_ASSERT (m_companyInfo.get() != NULL);
	}
	CCoGroupInfo(CCompanyInfoPointer companyInfo, const tstring & name, GroupType type)
		: m_companyInfo(companyInfo), m_groupid(0), m_name(name), m_type(type)
		, m_onlineCount(0)
	{
		BOOST_ASSERT (m_companyInfo.get() != NULL);
	}
	~CCoGroupInfo(void)
	{
		m_userinfos.clear();
	}

private:
	CCompanyInfoPointer m_companyInfo;
	unsigned long m_groupid;
	CCoGroupInfo::pointer m_parentgroup;
	tstring m_name;
	GroupType m_type;
	int m_onlineCount;
};

//typedef CCoGroupInfo::pointer CCoGroupInfoPointer;

#endif // __cogroupinfo_h__

