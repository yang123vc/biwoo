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

// companyinfo.h file here
#ifndef __companyinfo_h__
#define __companyinfo_h__

#include <stl/stldef.h>
//#include <stl/locklist.h>
//#include <stl/lockmap.h>
#include <boost/shared_ptr.hpp>
#include <CGCBase/cgcpointer.h>
#include "cogroupinfo.h"

class CCompanyInfo
	: public cgc::cgcPointer
{
public:
	typedef boost::shared_ptr<CCompanyInfo> pointer;

	static CCompanyInfo::pointer create(unsigned int coid, const std::string & name)
	{
		return CCompanyInfo::pointer(new CCompanyInfo(coid, name));
	}

	CCompanyInfo(unsigned int coid, const std::string & name)
		: m_companyId(coid), m_companyName(name)
		, m_onlineCount(0)
	{
	}
	~CCompanyInfo(void)
	{
		m_allgroups.clear();
		m_userinfos.clear();
	}

	CLockMap<unsigned int, CCoGroupInfoPointer> m_allgroups;			// All Groups.
	CLockMap<std::string, CUserInfoPointer>	m_userinfos;

	int onlineCount(void) const {return m_onlineCount;}
	int iOnlineCount(void) {return ++m_onlineCount;}
	int dOnlineCount(void) {return --m_onlineCount;}

	int getCoGroupUser(long cogroupId, bool bGetChild = true) const
	{
		CCoGroupInfo::pointer cogroupInfo;
		if (!m_allgroups.find(cogroupId, cogroupInfo)) return 0;

		int result = cogroupInfo->m_userinfos.size();

		if (bGetChild)
		{
			CLockMap<unsigned int, CCoGroupInfo::pointer>::const_iterator iter;
			//boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(m_allgroups.mutex()));
			for (iter=m_allgroups.begin(); iter!=m_allgroups.end(); iter++)
			{
				if (iter->second->parentgroup().get() != 0 &&
					iter->second->parentgroup()->groupid() == (unsigned long)cogroupId)
				{
					result += (int)iter->second->m_userinfos.size(); 
				}
			}
		}
		return result;
	}

	bool groupHasChildGroup(long groupId) const
	{
		CLockMap<unsigned int, CCoGroupInfo::pointer>::const_iterator iter;
		boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(m_allgroups.mutex()));
		for (iter=m_allgroups.begin(); iter!=m_allgroups.end(); iter++)
		{
			if (iter->second->parentgroup().get() != 0 &&
				iter->second->parentgroup()->groupid() == (unsigned long)groupId)
			{
				return true;
			}
		}
		return false;
	}

	bool hasCoGroup(unsigned int cogroupId)
	{
		CLockMap<unsigned int, CCoGroupInfo::pointer>::const_iterator iter;
		boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(m_allgroups.mutex()));
		for (iter=m_allgroups.begin(); iter!=m_allgroups.end(); iter++)
		{
			if (iter->second->groupid() == cogroupId)
			{
				return true;
			}
		}
		return false;
	}

	CUserInfoPointer getUserInfo(const std::string & userAccount)
	{
		CUserInfoPointer result;
		CLockMap<unsigned int, CCoGroupInfo::pointer>::const_iterator iter;
		boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(m_allgroups.mutex()));
		for (iter=m_allgroups.begin(); iter!=m_allgroups.end(); iter++)
		{
			CUserInfoPointer userInfo;
			if (iter->second->m_userinfos.find(userAccount, userInfo))
			{
				result = userInfo;
				break;;
			}
		}
		return result;
	}


public:
	unsigned int id(void) const {return m_companyId;}
	void name(const std::string & newv) {m_companyName = newv;}
	const std::string & name(void) const {return m_companyName;}

private:
	unsigned int m_companyId;
	std::string m_companyName;
	int m_onlineCount;

};


#endif // __companyinfo_h__

