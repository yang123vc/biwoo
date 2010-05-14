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

// frominfo.h file here
#ifndef __frominfo_h__
#define __frominfo_h__

#include <stl/stldef.h>
#include <boost/shared_ptr.hpp>
//#include "userinfo.h"
#include "dialoginfo.h"

class CUserInfo;
typedef boost::shared_ptr<CUserInfo> CUserInfoPointer;

class CCoGroupInfo;
typedef boost::shared_ptr<CCoGroupInfo> CCoGroupInfoPointer;

class CCompanyInfo;
typedef boost::shared_ptr<CCompanyInfo> CCompanyInfoPointer;

class CFromInfo
{
public:
	enum FromType
	{
		FromUserInfo
		, FromDialogInfo
		, FromCoGroup
		, FromCompany
	};

	typedef boost::shared_ptr<CFromInfo> pointer;
	static CFromInfo::pointer create(CUserInfoPointer fromUser)
	{
		return CFromInfo::pointer(new CFromInfo(fromUser));
	}
	static CFromInfo::pointer create(CDialogInfo::pointer fromDialog)
	{
		return CFromInfo::pointer(new CFromInfo(fromDialog));
	}
	static CFromInfo::pointer create(CCoGroupInfoPointer fromCoGroup)
	{
		return CFromInfo::pointer(new CFromInfo(fromCoGroup));
	}
	static CFromInfo::pointer create(CCompanyInfoPointer fromCompany)
	{
		return CFromInfo::pointer(new CFromInfo(fromCompany));
	}

	CFromInfo(CUserInfoPointer fromUser)
		: m_fromType(FromUserInfo), m_fromUser(fromUser)
	{
		BOOST_ASSERT (m_fromUser.get() != NULL);
	}
	CFromInfo(CDialogInfo::pointer fromDialog)
		: m_fromType(FromDialogInfo), m_fromDialog(fromDialog)
	{
		BOOST_ASSERT (m_fromDialog.get() != NULL);
	}
	CFromInfo(CCoGroupInfoPointer fromCoGroup)
		: m_fromType(FromCoGroup), m_fromCoGroup(fromCoGroup)
	{
		BOOST_ASSERT (m_fromCoGroup.get() != NULL);
	}
	CFromInfo(CCompanyInfoPointer fromCompany)
		: m_fromType(FromCompany), m_fromCompany(fromCompany)
	{
		BOOST_ASSERT (m_fromCompany.get() != NULL);
	}

	~CFromInfo(void)
	{
	}

public:
	FromType fromType(void) const {return m_fromType;}
	CUserInfoPointer fromUser(void) const {return m_fromUser;}
	CDialogInfo::pointer fromDialog(void) const {return m_fromDialog;}
	CCoGroupInfoPointer fromCoGroup(void) const {return m_fromCoGroup;}
	CCompanyInfoPointer fromCompany(void) const {return m_fromCompany;}

private:
	FromType m_fromType;
	CUserInfoPointer m_fromUser;
	CDialogInfo::pointer m_fromDialog;
	CCoGroupInfoPointer m_fromCoGroup;
	CCompanyInfoPointer m_fromCompany;

};


#endif // __frominfo_h__

