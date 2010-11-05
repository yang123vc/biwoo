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

// UserInfo.h file here
#ifndef __UserInfo_h__
#define __UserInfo_h__

#include <map>
#include <CGCBase/cgcobject.h>
#include "../ThirdParty/stl/stldef.h"
#include <boost/shared_ptr.hpp>

class CUserInfo
	: public cgc::cgcObject
	//: public cgc::cgcPointer
{
public:
	typedef boost::shared_ptr<CUserInfo> pointer;
	CUserInfo(const tstring & username, const tstring & password)
		: m_sUsername(username), m_sPassword(password)
	{}
	~CUserInfo(void)
	{}

	static CUserInfo::pointer create(const tstring & username, const tstring & password)
	{
		return CUserInfo::pointer(new CUserInfo(username, password));
	}

public:

	void setUsername(const tstring & newValue) {m_sUsername = newValue;}
	const tstring & getUsername(void) const {return m_sUsername;}

	void setPassword(const tstring & newValue) {m_sPassword = newValue;}
	const tstring & getPassword(void) const {return m_sPassword;}

	void setSessionId(const tstring & newValue) {m_sSessionId = newValue;}
	const tstring & getSessionId(void) const {return m_sSessionId;}

private:
	tstring m_sUsername;
	tstring m_sPassword;
	tstring m_sSessionId;	// For CGC
};


#endif // __UserInfo_h__

