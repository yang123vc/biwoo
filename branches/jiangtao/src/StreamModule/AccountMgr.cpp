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
#include <CGCBase/includeapp.h>
using namespace cgc;

///////////////////////////////
// 
#include "AVSProxy.h"

//////////////////////////
// AccountRegister
extern "C" int CGC_API AccountRegister(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	// Request
	cgcParameter::pointer pUserName = request->getParameter(_T("UserName"));
	if (pUserName.get() == 0 || pUserName->getValue().length() == 0) return -1;
	cgcParameter::pointer pPassword = request->getParameter(_T("Password"));
	if (pPassword.get() == 0) return -1;

	// addUserInfo
	//CUserInfo::pointer pUserInfo = (const CUserInfo::pointer&)gApplication->getAttribute(NAME_USERINFO, pUserName->getValue());
	CUserInfo::pointer pUserInfo = boost::static_pointer_cast<CUserInfo, cgcPointer>(gApplication->getAttribute(NAME_USERINFO, pUserName->getValue()));
	if (pUserInfo.get() == NULL)
	//if (!gAVSProxy.m_mapUserInfo.find(pUserName->getValue(), pUserInfo, false))
	{
		pUserInfo = CUserInfo::create(pUserName->getValue(), pPassword->getValue());
		gApplication->setAttribute(NAME_USERINFO, pUserInfo->getUsername(), pUserInfo);
		//gAVSProxy.m_mapUserInfo.insert(pUserInfo->getUsername(), pUserInfo);
	}else
	{
		pUserInfo->setUsername(pUserName->getValue());
		pUserInfo->setPassword(pPassword->getValue());
	}
	pUserInfo->setSessionId(session->getId());

	// Response
	response->setParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("UserName"), pUserInfo->getUsername()));
	response->sendResponse(0);

	return 0;
}


//////////////////////////
// AccountUnRegister
extern "C" int CGC_API AccountUnRegister(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	// UserName
	cgcParameter::pointer pUserName = request->getParameter(_T("UserName"));
	if (pUserName.get() == 0 || pUserName->getValue().length() == 0) return -1;

	CUserInfo::pointer pUserInfo = boost::static_pointer_cast<CUserInfo,cgcPointer>(gApplication->getAttribute(NAME_USERINFO, pUserName->getValue()));
	if (pUserInfo.get() == NULL)
//	if (!gAVSProxy.m_mapUserInfo.find(pUserName->getValue(), pUserInfo))
	{
		return -1;
	}

	if (session->getId().compare(pUserInfo->getSessionId()) != 0)
	{
		return -3;
	}
	gApplication->removeAttribute(NAME_USERINFO, pUserInfo->getUsername());
	//gAVSProxy.m_mapUserInfo.remove(pUserInfo->getUsername());
	
	return 0;
}
