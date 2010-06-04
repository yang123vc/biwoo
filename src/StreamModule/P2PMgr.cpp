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

// P2PMgr.cpp : Defines the initialization routines for the DLL.
//

#ifdef WIN32
#include <windows.h>
#endif

// cgc头文件
#include <CGCBase/includeapp.h>
using namespace cgc;

///////////////////////////////
// 
#include "AVSProxy.h"


//////////////////////////
// RequestP2PUser：请求一个P2P连接
extern "C" int CGC_API RequestP2PUser(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	// Request
	cgcParameter::pointer pFromUser = request->getParameter(_T("FromUser"));
	if (pFromUser.get() == 0 || pFromUser->getValue().length() == 0) return -1;
	cgcParameter::pointer pToUser = request->getParameter(_T("ToUser"));
	if (pToUser.get() == 0 || pToUser->getValue().length() == 0) return -1;
	long nP2PType = request->getParameterValue(_T("P2PType"), 0);
	long nP2PParam = request->getParameterValue(_T("P2PParam"), 0);

//	response->setParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FromUser"), pToUser->getValue()));
//	response->setParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("P2PType"), sP2PType));


	// Send P2P request
	CUserInfo::pointer pFromUserInfo = CGC_POINTER_CAST<CUserInfo>(gApplication->getAttribute(NAME_USERINFO, pFromUser->getValue()));
	if (pFromUserInfo.get() == NULL) return -2;
	//if (!gAVSProxy->m_mapUserInfo.find(pFromUser->getValue(), pFromUserInfo)) return -2;

	cgcSession::pointer pFromUserCgcSession = gSystem->getcgcSession(pFromUserInfo->getSessionId());
	if (pFromUserCgcSession.get() == NULL) return -2;
	cgcResponse::pointer pFromUserCgcResponse = pFromUserCgcSession->getLastResponse();
	if (pFromUserCgcResponse.get() == NULL || pFromUserCgcResponse->isInvalidate())
	{
		return -2;
	}

	pFromUserCgcResponse->setParameter(cgcParameter::create(_T("FromUser"), pToUser->getValue()));
	pFromUserCgcResponse->setParameter(cgcParameter::create(_T("P2PType"), nP2PType));
	pFromUserCgcResponse->setParameter(cgcParameter::create(_T("P2PParam"), nP2PParam));
	CUserInfo::pointer pToUserInfo = CGC_POINTER_CAST<CUserInfo>(gApplication->getAttribute(NAME_USERINFO, pToUser->getValue()));
	if (pToUserInfo.get() == NULL)
	//if (!gAVSProxy->m_mapUserInfo.find(pToUser->getValue(), pToUserInfo))
	{
		pFromUserCgcResponse->sendResponse(-3, 1006);
		return -3;
	}
	cgcSession::pointer pToUserCgcSession = gSystem->getcgcSession(pToUserInfo->getSessionId());
	if (pToUserCgcSession.get() == NULL)
	{
		pFromUserCgcResponse->sendResponse(-4, 1006);
		return -4;
	}
	cgcResponse::pointer pToUserCgcResponse = pToUserCgcSession->getLastResponse();
	if (pToUserCgcResponse.get() == NULL || pToUserCgcResponse->isInvalidate())
	{
		pFromUserCgcResponse->sendResponse(-4, 1006);
		return -4;
	}

	pFromUserCgcResponse->sendResponse(0, 1006);
//	response->sendResponse();

	// ???
	// Message:
	// 1002: P2P Request Message
	pToUserCgcResponse->setParameter(cgcParameter::create(_T("FromUser"), pFromUserInfo->getUsername()));
	pToUserCgcResponse->setParameter(cgcParameter::create(_T("P2PType"), nP2PType));
	pToUserCgcResponse->setParameter(cgcParameter::create(_T("P2PParam"), nP2PParam));
	pToUserCgcResponse->setParameter(cgcParameter::create(_T("RemoteAddr"), request->getRemoteAddr()));
	pToUserCgcResponse->sendResponse(0, 1002);
	return 0;
}

// ResponseP2PUser: 响应一个P2P请求
extern "C" int CGC_API ResponseP2PUser(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	// Request
	cgcParameter::pointer pFromUser = request->getParameter(_T("FromUser"));
	if (pFromUser == 0 || pFromUser->getValue().length() == 0) return -1;
	cgcParameter::pointer pToUser = request->getParameter(_T("ToUser"));
	if (pToUser == 0 || pToUser->getValue().length() == 0) return -1;
	long nP2PType = request->getParameterValue(_T("P2PType"), 0);
	long nP2PParam = request->getParameterValue(_T("P2PParam"), 0);

	// Send P2P Response
	CUserInfo::pointer pFromUserInfo = CGC_POINTER_CAST<CUserInfo>(gApplication->getAttribute(NAME_USERINFO, pFromUser->getValue()));
	if (pFromUserInfo.get() == NULL) return -2;
	//if (!gAVSProxy->m_mapUserInfo.find(pFromUser->getValue(), pFromUserInfo)) return -2;
	CUserInfo::pointer pToUserInfo = CGC_POINTER_CAST<CUserInfo>(gApplication->getAttribute(NAME_USERINFO, pToUser->getValue()));
	if (pToUserInfo.get() == NULL) return -2;
	//if (!gAVSProxy->m_mapUserInfo.find(pToUser->getValue(), pToUserInfo)) return -2;

	cgcSession::pointer pToUserCgcSession = gSystem->getcgcSession(pToUserInfo->getSessionId());
	if (pToUserCgcSession.get() == NULL) return -3;
	cgcResponse::pointer pToUserCgcResponse = pToUserCgcSession->getLastResponse();
	if (pToUserCgcResponse.get() == NULL || pToUserCgcResponse->isInvalidate())
	{
		return -3;
	}

	response->sendResponse();
//	response->sendResponse();
//	response->sendResponse();

	// ???
	// Message:
	// 1003: P2P Response Message
//	pToUserCgcResponse->setParameter(cgcParameter::create(cgcParameter::PT_INT, _T("MessageType"), _T("1003")));
	pToUserCgcResponse->setParameter(cgcParameter::create(_T("FromUser"), pFromUserInfo->getUsername()));
	pToUserCgcResponse->setParameter(cgcParameter::create(_T("P2PType"), nP2PType));
	pToUserCgcResponse->setParameter(cgcParameter::create(_T("P2PParam"), nP2PParam));
	pToUserCgcResponse->setParameter(cgcParameter::create(_T("RemoteAddr"), request->getRemoteAddr()));
	pToUserCgcResponse->sendResponse(0, 1003);

	cgcSession::pointer pFromUserCgcSession = gSystem->getcgcSession(pFromUserInfo->getSessionId());
	if (pFromUserCgcSession.get() == NULL) return -4;
	cgcResponse::pointer pFromUserCgcResponse = pFromUserCgcSession->getLastResponse();
	if (pFromUserCgcResponse.get() == NULL || pFromUserCgcResponse->isInvalidate())
	{
		return -4;
	}

	pFromUserCgcResponse->setParameter(cgcParameter::create(_T("ToUser"), pToUser->getValue()));
	pFromUserCgcResponse->setParameter(cgcParameter::create(_T("P2PType"), nP2PType));
	pFromUserCgcResponse->setParameter(cgcParameter::create(_T("P2PParam"), nP2PParam));
	pFromUserCgcResponse->sendResponse(0, 1004);
	return 0;
}

extern "C" int CGC_API DisconnectP2PUser(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	// Request
	cgcParameter::pointer pFromUser = request->getParameter(_T("FromUser"));
	if (pFromUser == 0 || pFromUser->getValue().length() == 0) return -1;
	cgcParameter::pointer pToUser = request->getParameter(_T("ToUser"));
	if (pToUser == 0 || pToUser->getValue().length() == 0) return -1;
	long nP2PType = request->getParameterValue(_T("P2PType"), 0);
	long nP2PParam = request->getParameterValue(_T("P2PParam"), 0);

	// Send P2P request
	CUserInfo::pointer pFromUserInfo = CGC_POINTER_CAST<CUserInfo>(gApplication->getAttribute(NAME_USERINFO, pFromUser->getValue()));
	if (pFromUser.get() == NULL) return -2;
	//if (!gAVSProxy->m_mapUserInfo.find(pFromUser->getValue(), pFromUserInfo)) return -2;
	CUserInfo::pointer pToUserInfo = CGC_POINTER_CAST<CUserInfo>(gApplication->getAttribute(NAME_USERINFO, pToUser->getValue()));
	if (pToUserInfo.get() == NULL) return -2;
	//if (!gAVSProxy->m_mapUserInfo.find(pToUser->getValue(), pToUserInfo)) return -2;

	cgcSession::pointer pToUserCgcSession = gSystem->getcgcSession(pToUserInfo->getSessionId());
	if (pToUserCgcSession.get() == NULL) return -3;
	cgcResponse::pointer pToUserCgcResponse = pToUserCgcSession->getLastResponse();
	if (pToUserCgcResponse.get() == NULL || pToUserCgcResponse->isInvalidate())
	{
		return -3;
	}

	// ???
	// Message:
	pToUserCgcResponse->setParameter(cgcParameter::create(_T("FromUser"), pFromUserInfo->getUsername()));
	pToUserCgcResponse->setParameter(cgcParameter::create(_T("P2PType"), nP2PType));
	pToUserCgcResponse->setParameter(cgcParameter::create(_T("P2PParam"), nP2PParam));
	pToUserCgcResponse->sendResponse(0, 1005);

	response->setParameter(cgcParameter::create(_T("FromUser"), pToUser->getValue()));
	response->setParameter(cgcParameter::create(_T("P2PType"), nP2PType));
	response->setParameter(cgcParameter::create(_T("P2PParam"), nP2PParam));
	return 0;
}

// NotifyP2PUser: 通知P2P用户事件
//extern "C" int CGC_API NotifyP2PUser(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
//{
//	return 0;
//}

