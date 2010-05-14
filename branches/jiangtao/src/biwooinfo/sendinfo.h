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

// sendinfo.h file here
#ifndef __sendinfo_h__
#define __sendinfo_h__

#include <stl/stldef.h>
//#include <stl/lockmap.h>
#include <boost/shared_ptr.hpp>
#include "messageinfo.h"

class CSendInfo
{
public:
	typedef boost::shared_ptr<CSendInfo> pointer;
	static CSendInfo::pointer create(CUserInfo::pointer fromUser, CMessageInfo::pointer messageinfo)
	{
		return CSendInfo::pointer(new CSendInfo(fromUser, messageinfo));
	}

	CSendInfo(CUserInfo::pointer fromUser, CMessageInfo::pointer messageinfo)
		: m_fromUser(fromUser), m_messageinfo(messageinfo)
		, m_responsed(false), m_accepted(false), m_canceled(false)
		, m_filepath(""), m_filename("")
	{
		BOOST_ASSERT (m_fromUser.get() != NULL);
		BOOST_ASSERT (m_messageinfo.get() != NULL);
	}
	~CSendInfo(void)
	{
		m_filestream.close();
	}


	//CLockMap<unsigned long, bool>	m_indexs;

public:
	CUserInfo::pointer fromUser(void) const {return m_fromUser;}
	CMessageInfo::pointer msginfo(void) const {return m_messageinfo;}

	bool responsed(void) const {return m_responsed;}
	void accept(bool newv) {m_accepted = newv; m_responsed = true;}
	bool accepted(void) const {return m_accepted;}

	void cancel(void) {m_canceled = true;}
	bool canceled(void) const {return m_canceled;}

	void filepath(const std::string & newv) {m_filepath = newv;}
	const std::string & filepath(void) const {return m_filepath;}
	void filename(const std::string & newv) {m_filename = newv;}
	const std::string & filename(void) const {return m_filename;}

	tfstream & fs(void) {return m_filestream;}

private:
	CUserInfo::pointer m_fromUser;
	CMessageInfo::pointer m_messageinfo;
	bool m_responsed;
	bool m_accepted;
	bool m_canceled;
	std::string m_filepath;
	std::string m_filename;
	tfstream m_filestream;

};


#endif // __sendinfo_h__