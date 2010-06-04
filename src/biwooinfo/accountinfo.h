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

// accountinfo.h file here
#ifndef __accountinfo_h__
#define __accountinfo_h__

#include <ThirdParty/stl/stldef.h>
#include <ThirdParty/stl/locklist.h>
#include <ThirdParty/stl/lockmap.h>
#include <boost/shared_ptr.hpp>
#include <CGCBase/cgcpointer.h>
#include "groupinfo.h"
#include "cogroupinfo.h"
#include "friendinfo.h"
#include "datagroup.h"
#include "datainfo.h"

class CAccountInfo
	: public cgc::cgcPointer
{
public:
	typedef boost::shared_ptr<CAccountInfo> pointer;

	static CAccountInfo::pointer create(CUserInfo::pointer userinfo, const tstring & accountId)
	{
		return CAccountInfo::pointer(new CAccountInfo(userinfo, accountId));
	}

	CAccountInfo(CUserInfo::pointer userinfo, const tstring & accountId)
		: m_userinfo(userinfo), m_sAccountId(accountId)
		, m_sSessionId(_T(""))
		, m_curgroupid(0), m_curdatagroupid(0), m_curdataid(0)
	{
		BOOST_ASSERT (m_userinfo.get() != 0);
	}
	~CAccountInfo(void)
	{
		m_allfriends.clear();
		m_allgroups.clear();
		m_datagroups.clear();
		m_datas.clear();
		m_cogroups.clear();
		//m_cogroups.clear();
		//m_members.clear();
		//m_groups.clear();
	}

	// Friends
	CLockMap<tstring, CFriendInfo::pointer> m_allfriends;		// All friends
	CLockMap<long, CGroupInfo::pointer> m_allgroups;			// All Groups.

	// Datas
	CLockMap<long, CDataGroup::pointer> m_datagroups;			// Data Groups.
	CLockMap<long, CDataInfo::pointer> m_datas;					// All Datas.
	CLockMap<unsigned int, CCoGroupInfo::pointer> m_cogroups;	// All CoGroup.
	
	//CLockMap<tstring, CFriendInfo::pointer> m_members;			// Not group friends
	//CLockMap<long, CGroupInfo::pointer> m_groups;

	bool groupHasChildGroup(long groupId) const
	{
		CLockMap<long, CGroupInfo::pointer>::const_iterator iter;
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
	bool groupHasChildFriend(long groupId) const
	{
		CLockMap<tstring, CFriendInfo::pointer>::const_iterator iter;
		boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(m_allfriends.mutex()));
		for (iter=m_allfriends.begin(); iter!=m_allfriends.end(); iter++)
		{
			if (iter->second->parentgroup().get() != 0 &&
				iter->second->parentgroup()->groupid() == (unsigned long)groupId)
			{
				return true;
			}
		}
		return false;
	}

	bool datagroupHasChildGroup(long groupId) const
	{
		CLockMap<long, CDataGroup::pointer>::const_iterator iter;
		boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(m_datagroups.mutex()));
		for (iter=m_datagroups.begin(); iter!=m_datagroups.end(); iter++)
		{
			if (iter->second->parentgroup().get() != 0 &&
				iter->second->parentgroup()->groupid() == groupId)
			{
				return true;
			}
		}
		return false;
	}

	bool datagroupHasChildData(long groupId) const
	{
		CLockMap<long, CDataInfo::pointer>::const_iterator iter;
		boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(m_datas.mutex()));
		for (iter=m_datas.begin(); iter!=m_datas.end(); iter++)
		{
			if (iter->second->parentgroup().get() != 0 &&
				iter->second->parentgroup()->groupid() == groupId)
			{
				return true;
			}
		}
		return false;
	}

public:
	CUserInfo::pointer getUserinfo(void) const {return m_userinfo;}

	void setAccountId(const tstring & newv) {m_sAccountId = newv;}
	const tstring & getAccountId(void) const {return m_sAccountId;}
	void setSessionId(const tstring & newValue) {m_sSessionId = newValue;}
	const tstring & getSessionId(void) const {return m_sSessionId;}

	void setcurgroupid(long newv) {m_curgroupid = newv;}
	long getcurgroupid(void) const {return m_curgroupid;}
	long getnextgroupid(void) {return ++m_curgroupid;}

	void setcurdatagroupid(long newv) {m_curdatagroupid = newv;}
	long getcurdatagroupid(void) const {return m_curdatagroupid;}
	long getnextdatagroupid(void) {return ++m_curdatagroupid;}

	void setcurdataid(long newv) {m_curdataid = newv;}
	long getcurdataid(void) const {return m_curdataid;}
	long getnextdataid(void) {return ++m_curdataid;}

private:
	CUserInfo::pointer m_userinfo;
	tstring m_sAccountId;	// Account@DOMAIN
	tstring m_sSessionId;	// For CGC

	long m_curgroupid;
	long m_curdatagroupid;
	long m_curdataid;
};


#endif // __accountinfo_h__

