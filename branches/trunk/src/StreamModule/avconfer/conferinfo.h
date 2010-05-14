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

// conferinfo.h file here
#ifndef __conferinfo_h__
#define __conferinfo_h__

#include <map>
#include <stl/stldef.h>
#include <boost/shared_ptr.hpp>

class CConferInfo
{
public:
	typedef boost::shared_ptr<CConferInfo> pointer;
	static CConferInfo::pointer create(const tstring & name, unsigned short maxNumbers)
	{
		return CConferInfo::pointer(new CConferInfo(name, maxNumbers));
	}
	static CConferInfo::pointer create(void)
	{
		return CConferInfo::pointer(new CConferInfo());
	}
public:
	void name(const tstring & newv) {m_name = newv;}
	const tstring & name(void) const {return m_name;}

	void maxNumbers(unsigned short newv) {m_maxNumbers = newv;}
	unsigned short maxNumbers(void) const {return m_maxNumbers;}

public:
	CConferInfo(void)
		: m_name(_T("")), m_maxNumbers(0)
	{}
	CConferInfo(const tstring & name, unsigned short maxNumbers)
		: m_name(name), m_maxNumbers(maxNumbers)
	{}
	~CConferInfo(void)
	{}


private:
	tstring m_name;
	unsigned short m_maxNumbers;
};


#endif // __conferinfo_h__

