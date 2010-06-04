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

// datainfo.h file here
#ifndef __datainfo_h__
#define __datainfo_h__

#include <ThirdParty/stl/stldef.h>
#include <ThirdParty/stl/locklist.h>
#include <boost/shared_ptr.hpp>
#include "datagroup.h"

class CDataInfo
{
public:
	typedef boost::shared_ptr<CDataInfo> pointer;
	static CDataInfo::pointer create(long dataid, const tstring & name, const tstring & desc, CDataGroup::pointer group)
	{
		return CDataInfo::pointer(new CDataInfo(dataid, name, desc, group));
	}

	CDataInfo(long dataid, const tstring & name, const tstring & desc, CDataGroup::pointer group)
		: m_dataid(dataid), m_name(name), m_desc(desc), m_group(group)
	{}
	~CDataInfo(void)
	{}

public:
	long dataid(void) const {return m_dataid;}

	void name(const tstring & newv) {m_name = newv;}
	const tstring & name(void) const {return m_name;}

	void desc(const tstring & newv) {m_desc = newv;}
	const tstring & desc(void) const {return m_desc;}

	void parentgroup(CDataGroup::pointer newv) {BOOST_ASSERT (newv.get() != 0); m_group = newv;}
	CDataGroup::pointer parentgroup(void) const {return m_group;}

private:
	long m_dataid;
	tstring m_name;
	tstring m_desc;
	CDataGroup::pointer m_group;
};


#endif // __datainfo_h__

