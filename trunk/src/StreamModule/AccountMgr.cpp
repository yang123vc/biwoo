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

// AccountMgr.cpp : Defines the initialization routines for the DLL.
//

#ifdef WIN32
#include <windows.h>
#endif

// cgcÍ·ÎÄ¼þ
#include <CGCBase/sotpapp.h>
using namespace cgc;

///////////////////////////////
// 
#include "AVSProxy.h"

//////////////////////////
// AccountRegister
extern "C" int CGC_API AccountRegister(const cgcSotpRequest::pointer & request, cgcSotpResponse::pointer response)
{
	// Request
	cgcParameter::pointer pUserName = request->getParameter(_T("UserName"));
	if (pUserName.get() == 0 || pUserName->empty()) return -1;
	cgcParameter::pointer pPassword = request->getParameter(_T("Password"));
	if (pPassword.get() == 0) return -1;

	// addUserInfo
	cgcAttributes::pointer attributes = theApplication->getAttributes(true);
	CUserInfo::pointer pUserInfo = boost::static_pointer_cast<CUserInfo, cgcObject>(attributes->getAttribute(NAME_USERINFO, pUserName->getStr()));
	if (pUserInfo.get() == NULL)
	{
		pUserInfo = CUserInfo::create(pUserName->getStr(), pPassword->getStr());
		attributes->setAttribute(NAME_USERINFO, pUserInfo->getUsername(), pUserInfo);
	}else
	{
		pUserInfo->setUsername(pUserName->getStr());
		pUserInfo->setPassword(pPassword->getStr());
	}
	pUserInfo->setSessionId(request->getSession()->getId());

	// Response
	response->addParameter(CGC_PARAMETER(_T("UserName"), pUserInfo->getUsername()));
	response->sendResponse(0);

	return 0;
}


//////////////////////////
// AccountUnRegister
extern "C" int CGC_API AccountUnRegister(const cgcSotpRequest::pointer & request, cgcSotpResponse::pointer response)
{
	// UserName
	cgcParameter::pointer pUserName = request->getParameter(_T("UserName"));
	if (pUserName.get() == 0 || pUserName->empty()) return -1;

	cgcAttributes::pointer attributes = theApplication->getAttributes(true);
	CUserInfo::pointer pUserInfo = boost::static_pointer_cast<CUserInfo,cgcObject>(attributes->getAttribute(NAME_USERINFO, pUserName->getStr()));
	if (pUserInfo.get() == NULL)
	{
		return -1;
	}

	if (request->getSession()->getId().compare(pUserInfo->getSessionId()) != 0)
	{
		return -3;
	}
	attributes->removeAttribute(NAME_USERINFO, pUserInfo->getUsername());
	
	return 0;
}
