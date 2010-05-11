/*
    StreamModule is a P2P and conference communication module base on CGCP.
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

// xmlparseconfers.h file here
#ifndef __xmlparseconfers_h__
#define __xmlparseconfers_h__

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
using boost::property_tree::ptree;
#include "../ThirdParty/stl/lockmap.h"
#include "avconfer/conferinfo.h"

class xmlparseconfers
{
public:
	xmlparseconfers(void)
	{}
	~xmlparseconfers(void)
	{
		m_confers.clear();
	}

	CLockMap<tstring, CConferInfo::pointer> m_confers;

public:
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
		if (v.first.compare("confer") == 0)
		{
			int disable = v.second.get("disable", 0);
			if (disable == 1) return;

			std::string name = v.second.get("name", "");
			if (name.empty()) return;

			unsigned short maxNumber = v.second.get("maxnumbers", 0);

			CConferInfo::pointer conferInfo = CConferInfo::create(name, maxNumber);
			m_confers.insert(name, conferInfo);
		}
	}

};

#endif // __xmlparseconfers_h__
