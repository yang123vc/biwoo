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

// xmlparseusers.h file here
#ifndef __xmlparseusers_h__
#define __xmlparseusers_h__

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
using boost::property_tree::ptree;
#include <stl/lockmap.h>
#include "../biwooinfo/userinfo.h"

class xmlparseusers
{
public:
	xmlparseusers(void)
	{}
	~xmlparseusers(void)
	{
		m_users.clear();
	}

	CLockMap<tstring, CUserInfo::pointer> m_users;

public:
	//bool isAllowModule(const tstring & moduleName) const
	//{
	//	return getModuleItem(moduleName).get() != NULL;
	//}
	//bool getAllowMethod(const tstring & moduleName, const tstring & invokeName, tstring & methodName)
	//{
	//	ModuleItem::pointer result;
	//	if (!m_modules.find(moduleName, result))
	//		return false;
	//	return result->getAllowMethod(invokeName, methodName);
	//}

	//ModuleItem::pointer getModuleItem(const tstring & appName) const
	//{
	//	ModuleItem::pointer result;
	//	m_modules.find(appName, result);
	//	return result;
	//}
	//bool isModuleItem(const ModuleItem::pointer & pModuleItem) const;

	void load(const tstring & filename)
	{
		ptree pt;
		read_xml(filename, pt);

		try
		{
			BOOST_FOREACH(const ptree::value_type &v, pt.get_child("root"))
				Insert(v);

		}catch(...)
		{}
	}

private:
	void Insert(const boost::property_tree::ptree::value_type & v)
	{
		if (v.first.compare("user") == 0)
		{
			//ModuleItem::pointer moduleItem = ModuleItem::create();
			std::string account = v.second.get("account", "");
			if (account.empty()) return;

			std::string password = v.second.get("password", "");
			CUserInfo::pointer userInfo = CUserInfo::create(account, password);
			m_users.insert(account, userInfo);
		}
	}

};

#endif // __xmlparseusers_h__
