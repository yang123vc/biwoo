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

// AVStreamModule.cpp : Defines the initialization routines for the DLL.
//

#ifdef WIN32
#include <windows.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

// cgcÍ·ÎÄ¼þ
#include <CGCBase/includeapp.h>
#include <CGCBase/cgcString.h>
#include <CGCBase/cgcRtp.h>
#include <CGCBase/cgcSip.h>
using namespace cgc;

///////////////////////////////
// 
#include "AVSProxy.h"
#include "xmlparseconfers.h"

extern "C" bool CGC_API CGC_Module_Init(void)
{
	cgcRtp::pointer rtpService = CGC_RTPSERVICE_DEF(gCgcService->getService("RtpService"));
	cgcSip::pointer sipService = CGC_SIPSERVICE_DEF(gCgcService->getService("SipService"));
	BOOST_ASSERT (rtpService.get() != NULL);
	BOOST_ASSERT (sipService.get() != NULL);

	gAVSProxy = CAVSProxy::create(rtpService, sipService);

	bool result = true;
	bool enableSipConference = gApplication->getInitParameterValue2(_T("EnableSipConference"), false);
	if (enableSipConference)
	{
		// Load the setting conferinfo.
		tstring sXmlPath(gApplication->getAppConfPath());
		sXmlPath.append(_T("/default_confers.xml"));
		xmlparseconfers parse;
		parse.load(sXmlPath);

		CLockMap<tstring, CConferInfo::pointer>::iterator iter;
		for (iter=parse.m_confers.begin(); iter!=parse.m_confers.end(); iter++)
		{
			gAVSProxy->m_conference.addConference(iter->second);
		}

		CSipParameter sipp;
		sipp.ua(gApplication->getInitParameterValue(_T("UA"), _T("y")));
		sipp.pwd(gApplication->getInitParameterValue(_T("PWD"), _T("y")));
		sipp.identity(gApplication->getInitParameterValue(_T("IDENTITY"), _T("sip:y@192.168.19.77")));
		sipp.proxy(gApplication->getInitParameterValue(_T("PROXY"), _T("sip:192.168.19.84:5060")));
		sipp.sipport(gApplication->getInitParameterValue(_T("SIPPORT"), 5060));

		result = gAVSProxy->m_conference.initsip(sipp);
		gApplication->log(DL_INFO, _T("Conference Service SIPPORT: %d, %s"), sipp.sipport(), result ? _T("Succeed") : _T("Failed"));
	}
	return result;
}

extern "C" void CGC_API CGC_Module_Free(void)
{
	gAVSProxy->m_conference.quitsip();

	//CCommConferenceMap & mapCommConfer = const_cast<CCommConferenceMap&>(gAVSProxy.m_commfMgr.getMap());
	//CCommConferenceMapIter pIter;
	//for (pIter=mapCommConfer.begin(); pIter!=mapCommConfer.end(); pIter++)
	//{
	//	CCommConference * pCommConference = pIter->second;
	//	gSystem->CloseCommApp(pCommConference->getCommId());
	//}

	gApplication->clearAllAtrributes();
	gAVSProxy.reset();
}

extern "C" int CGC_API LoadSetting(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	// Request

	response->setParameter(gApplication->getInitParameter("P2PRTPSERVER"));
	response->setParameter(gApplication->getInitParameter("P2PUDPSERVER"));
	response->sendResponse();

	return 0;
}

/*
//////////////////////////
// UserLogin
extern "C" int CGC_API UserLogin(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	// Request
	cgcParameter::pointer pUserName = request->getParameter(_T("UserName"));
	if (pUserName.get() == 0 || pUserName->getValue().length() == 0) return -1;
	cgcParameter::pointer pPassword = request->getParameter(_T("Password"));
	if (pPassword.get() == 0) return -1;

	// addUserInfo
	CUserInfo::pointer pUserInfo = (const CUserInfo::pointer&)gApplication->getAttribute(NAME_USERINFO, pUserName->getValue());
	if (pUserInfo.get() == NULL)
	//if (!gAVSProxy.m_mapUserInfo.find(pUserName->getValue(), pUserInfo))
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
	response->sendResponse(1);

	// User Login Notify:
	boost::mutex::scoped_lock lock(gAVSProxy.m_mapUserInfo.mutex());
	const CLockMap<tstring, CUserInfo::pointer> & userInfoMap = gAVSProxy.m_mapUserInfo;
	CLockMap<tstring, CUserInfo::pointer>::const_iterator pIter;
	for (pIter=userInfoMap.begin(); pIter!=userInfoMap.end(); pIter++)
	{
		CUserInfo::pointer pFromUserInfo = pIter->second;
//		if (pFromUserInfo->getUsername().compare(pUserInfo->getUsername()) == 0)
//			continue;

		cgcSession::pointer pToUserCgcSession = gSystem->getcgcSession(pFromUserInfo->getSessionId());
		if (pToUserCgcSession.get() == NULL) continue;
		cgcResponse::pointer pToUserCgcResponse = pToUserCgcSession->getLastResponse();
		if (pToUserCgcResponse.get() == NULL || pToUserCgcResponse->isInvalidate())
		{
			continue;
		}

		// ???
		// Message:
		// 2001: User login notify
//		pToUserCgcResponse->setParameter(Parameter(Parameter::PT_INT, _T("MessageType"), _T("2001")));
		pToUserCgcResponse->setParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FromUser"), pUserInfo->getUsername()));
		pToUserCgcResponse->sendResponse(0, 2001);
	}

	return 0;
}


//////////////////////////
// UserLogout
extern "C" int CGC_API UserLogout(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	// UserName
	cgcParameter::pointer pUserName = request->getParameter(_T("UserName"));
	if (pUserName == 0 || pUserName->getValue().length() == 0) return -1;

	CUserInfo::pointer pUserInfo;
	if (!gAVSProxy.m_mapUserInfo.find(pUserName->getValue(), pUserInfo, true)) return -2;
	
	// User Logout Notify:
	boost::mutex::scoped_lock lock(gAVSProxy.m_mapUserInfo.mutex());
	const CLockMap<tstring, CUserInfo::pointer> & userInfoMap = gAVSProxy.m_mapUserInfo;
	CLockMap<tstring, CUserInfo::pointer>::const_iterator pIter;
	for (pIter=userInfoMap.begin(); pIter!=userInfoMap.end(); pIter++)
	{
		CUserInfo::pointer pFromUserInfo = pIter->second;

		cgcSession::pointer pToUserCgcSession = gSystem->getcgcSession(pFromUserInfo->getSessionId());
		if (pToUserCgcSession.get() == NULL) continue;
		cgcResponse::pointer pToUserCgcResponse = pToUserCgcSession->getLastResponse();
		if (pToUserCgcResponse.get() == NULL || pToUserCgcResponse->isInvalidate())
		{
			continue;
		}

		// ???
		// Message:
		// 2002: User logout notify
//		pToUserCgcResponse->setParameter(Parameter(Parameter::PT_INT, _T("MessageType"), _T("2002")));
		pToUserCgcResponse->setParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FromUser"), pUserName->getValue()));
		pToUserCgcResponse->sendResponse(0, 2002);
	}

	return 0;
}
*/
//////////////////////////
// GetAllUser
extern "C" int CGC_API GetAllUser(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	// Request
	cgcParameter::pointer pFromUser = request->getParameter(_T("FromUser"));
	if (pFromUser.get() == 0 || pFromUser->getValue().length() == 0) return -1;

	StringPointerMapPointer mapUserInfo = gApplication->getStringAttributes(NAME_USERINFO, false);
	if (mapUserInfo.get() != NULL)
	{
		boost::mutex::scoped_lock lock(mapUserInfo->mutex());
		CPointerMap<tstring>::iterator pIter;
		for (pIter=mapUserInfo->begin(); pIter!=mapUserInfo->end(); pIter++)
		{
			CUserInfo::pointer pUserInfo = (const CUserInfo::pointer&)pIter->second;

			// Response
			response->lockResponse();
			response->setParameter(cgcParameter::create(_T("UserId"), cgcString::Format(_T("%d"), (ULONG)pUserInfo.get())));
			response->setParameter(cgcParameter::create(_T("FromUser"), pUserInfo->getUsername()));
			response->sendResponse();
#ifdef WIN32
			Sleep(5);
#else
			usleep(5000);
#endif
		}
	}

	/*
	boost::mutex::scoped_lock lock(gAVSProxy.m_mapUserInfo.mutex());
	const CLockMap<tstring, CUserInfo::pointer> & userInfoMap = gAVSProxy.m_mapUserInfo;
	CLockMap<tstring, CUserInfo::pointer>::const_iterator pIter;
	for (pIter=userInfoMap.begin(); pIter!=userInfoMap.end(); pIter++)
	{
		CUserInfo::pointer pUserInfo = pIter->second;
	//	if (pUserInfo->getUsername().compare(pFromUser->getValue()) == 0)
	//		continue;

		// Response
		response->setParameter(cgcParameter::create(_T("UserId"), cgcString::Format(_T("%d"), (ULONG)pUserInfo.get())));
		response->setParameter(cgcParameter::create(_T("FromUser"), pUserInfo->getUsername()));
		response->sendResponse();
#ifdef WIN32
		Sleep(100);
#else
		usleep(100000);
#endif
	}
	*/

	return 1;
}
/*
extern "C" int CGC_API SendTextMessage(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	// Request
	cgcParameter::pointer pFromUser = request->getParameter(_T("FromUser"));
	if (pFromUser.get() == 0 || pFromUser->getValue().length() == 0) return -1;
	cgcParameter::pointer pToUser = request->getParameter(_T("ToUser"));
	if (pToUser.get() == 0 || pToUser->getValue().length() == 0) return -1;
	cgcParameter::pointer pSendMessage = request->getParameter(_T("Msg"));
	if (pSendMessage.get() == 0 || pSendMessage->getValue().length() == 0) return -1;

	// Send Text Message
	CUserInfo::pointer pFromUserInfo;
	if (!gAVSProxy.m_mapUserInfo.find(pFromUser->getValue(), pFromUserInfo)) return -2;
	CUserInfo::pointer pToUserInfo;
	if (!gAVSProxy.m_mapUserInfo.find(pToUser->getValue(), pToUserInfo)) return -2;

	cgcSession::pointer pToUserCgcSession = gSystem->getcgcSession(pToUserInfo->getSessionId());
	if (pToUserCgcSession.get() == NULL) return -3;
	cgcResponse::pointer pToUserCgcResponse = pToUserCgcSession->getLastResponse();
	if (pToUserCgcResponse.get() == NULL || pToUserCgcResponse->isInvalidate())
	{
		return -3;
	}

	// ???
	// Message:
	// 1001: Text Message
//	pToUserCgcResponse->setParameter(Parameter(Parameter::PT_INT, _T("MessageType"), _T("1001")));
	pToUserCgcResponse->setParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FromUser"), pFromUserInfo->getUsername()));
	pToUserCgcResponse->setParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("Msg"), pSendMessage->getValue()));
	pToUserCgcResponse->sendResponse(0, 1001);

	// Response
	return 0;
}
*/
