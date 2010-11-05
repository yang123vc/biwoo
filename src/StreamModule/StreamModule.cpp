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
#include <CGCBase/app.h>
//#include <CGCBase/cgcString.h>
#include <CGCServices/Rtp/cgcRtp.h>
#include <CGCServices/Sip/cgcSip.h>
using namespace cgc;

///////////////////////////////
// 
#include "AVSProxy.h"
#include "xmlparseconfers.h"

extern "C" bool CGC_API CGC_Module_Init(void)
{
	cgcParameterMap::pointer initParameters = theApplication->getInitParameters();

	tstring rtpServiceName = initParameters->getParameterValue("RtpServiceName", "RtpService");
	tstring sipServiceName = initParameters->getParameterValue("SipServiceName", "SipService");

	cgcRtp::pointer rtpService = CGC_RTPSERVICE_DEF(theServiceManager->getService(rtpServiceName));
	cgcSip::pointer sipService = CGC_SIPSERVICE_DEF(theServiceManager->getService(sipServiceName));
	BOOST_ASSERT (rtpService.get() != NULL);
	BOOST_ASSERT (sipService.get() != NULL);

	gAVSProxy = CAVSProxy::create(rtpService, sipService);

	bool result = true;
	//bool enableSipConference = initParameters->getParameterValue(_T("EnableSipConference"), false);
	//if (enableSipConference)
	//{
	//	// Load the setting conferinfo.
	//	tstring sXmlPath(theApplication->getAppConfPath());
	//	sXmlPath.append(_T("/default_confers.xml"));
	//	xmlparseconfers parse;
	//	parse.load(sXmlPath);

	//	CLockMap<tstring, CConferInfo::pointer>::iterator iter;
	//	for (iter=parse.m_confers.begin(); iter!=parse.m_confers.end(); iter++)
	//	{
	//		gAVSProxy->m_conference.addConference(iter->second);
	//	}

	//	CSipParameter sipp;
	//	sipp.ua(initParameters->getParameterValue(_T("UA"), _T("y")));
	//	sipp.pwd(initParameters->getParameterValue(_T("PWD"), _T("y")));
	//	sipp.identity(initParameters->getParameterValue(_T("IDENTITY"), _T("sip:y@192.168.19.77")));
	//	sipp.proxy(initParameters->getParameterValue(_T("PROXY"), _T("sip:192.168.19.84:5060")));
	//	sipp.sipport(initParameters->getParameterValue(_T("SIPPORT"), 5060));

	//	result = gAVSProxy->m_conference.initsip(sipp);
	//	theApplication->log(cgc::LOG_INFO, _T("Conference Service SIPPORT: %d, %s"), sipp.sipport(), result ? _T("Succeed") : _T("Failed"));
	//}
	return result;
}

extern "C" void CGC_API CGC_Module_Free(void)
{
	gAVSProxy->m_conference.quitsip();

	//CCommConferenceMap & mapCommConfer = const_cast<CCommConferenceMap&>(gAVSProxy->m_commfMgr.getMap());
	//CCommConferenceMapIter pIter;
	//for (pIter=mapCommConfer.begin(); pIter!=mapCommConfer.end(); pIter++)
	//{
	//	CCommConference * pCommConference = pIter->second;
	//	gSystem->CloseCommApp(pCommConference->getCommId());
	//}

	gAVSProxy.reset();
}

extern "C" int CGC_API LoadSetting(const cgcSotpRequest::pointer & request, cgcSotpResponse::pointer response)
{
	// Request
	cgcParameterMap::pointer initParameters = theApplication->getInitParameters();
	response->addParameter(initParameters->getParameter("P2PRTPSERVER"));
	response->addParameter(initParameters->getParameter("P2PUDPSERVER"));
	response->sendResponse();

	return 0;
}

/*
//////////////////////////
// UserLogin
extern "C" int CGC_API UserLogin(const cgcRequest::pointer & request, cgcResponse::pointer response)
{
	// Request
	cgcParameter::pointer pUserName = request->getParameter(_T("UserName"));
	if (pUserName.get() == 0 || pUserName->getValue().length() == 0) return -1;
	cgcParameter::pointer pPassword = request->getParameter(_T("Password"));
	if (pPassword.get() == 0) return -1;

	// addUserInfo
	CUserInfo::pointer pUserInfo = (const CUserInfo::pointer&)gApplication->getAttribute(NAME_USERINFO, pUserName->getValue());
	if (pUserInfo.get() == NULL)
	//if (!gAVSProxy->m_mapUserInfo.find(pUserName->getValue(), pUserInfo))
	{
		pUserInfo = CUserInfo::create(pUserName->getValue(), pPassword->getValue());
		gApplication->setAttribute(NAME_USERINFO, pUserInfo->getUsername(), pUserInfo);
		//gAVSProxy->m_mapUserInfo.insert(pUserInfo->getUsername(), pUserInfo);
	}else
	{
		pUserInfo->setUsername(pUserName->getValue());
		pUserInfo->setPassword(pPassword->getValue());
	}
	pUserInfo->setSessionId(session->getId());

	// Response
	response->addParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("UserName"), pUserInfo->getUsername()));
	response->sendResponse(1);

	// User Login Notify:
	boost::mutex::scoped_lock lock(gAVSProxy->m_mapUserInfo.mutex());
	const CLockMap<tstring, CUserInfo::pointer> & userInfoMap = gAVSProxy->m_mapUserInfo;
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
//		pToUserCgcResponse->addParameter(Parameter(Parameter::PT_INT, _T("MessageType"), _T("2001")));
		pToUserCgcResponse->addParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FromUser"), pUserInfo->getUsername()));
		pToUserCgcResponse->sendResponse(0, 2001);
	}

	return 0;
}


//////////////////////////
// UserLogout
extern "C" int CGC_API UserLogout(const cgcRequest::pointer & request, cgcResponse::pointer response)
{
	// UserName
	cgcParameter::pointer pUserName = request->getParameter(_T("UserName"));
	if (pUserName == 0 || pUserName->getValue().length() == 0) return -1;

	CUserInfo::pointer pUserInfo;
	if (!gAVSProxy->m_mapUserInfo.find(pUserName->getValue(), pUserInfo, true)) return -2;
	
	// User Logout Notify:
	boost::mutex::scoped_lock lock(gAVSProxy->m_mapUserInfo.mutex());
	const CLockMap<tstring, CUserInfo::pointer> & userInfoMap = gAVSProxy->m_mapUserInfo;
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
//		pToUserCgcResponse->addParameter(Parameter(Parameter::PT_INT, _T("MessageType"), _T("2002")));
		pToUserCgcResponse->addParameter(cgcParameter::create(cgcParameter::PT_STRING, _T("FromUser"), pUserName->getValue()));
		pToUserCgcResponse->sendResponse(0, 2002);
	}

	return 0;
}
*/
//////////////////////////
// GetAllUser
extern "C" int CGC_API GetAllUser(const cgcSotpRequest::pointer & request, cgcSotpResponse::pointer response)
{
	// Request
	cgcParameter::pointer pFromUser = request->getParameter(_T("FromUser"));
	if (pFromUser.get() == 0 || pFromUser->empty()) return -1;

	cgcAttributes::pointer attributes = theApplication->getAttributes(true);

	StringObjectMapPointer mapUserInfo = attributes->getStringAttributes(NAME_USERINFO, false);
	if (mapUserInfo.get() != NULL)
	{
		boost::mutex::scoped_lock lock(mapUserInfo->mutex());
		CObjectMap<tstring>::iterator pIter;
		for (pIter=mapUserInfo->begin(); pIter!=mapUserInfo->end(); pIter++)
		{
			CUserInfo::pointer pUserInfo = (const CUserInfo::pointer&)pIter->second;

			// Response
			response->lockResponse();
			char buffer[20];
			sprintf(buffer, "%d", (ULONG)pUserInfo.get());

			response->addParameter(CGC_PARAMETER(_T("UserId"), buffer));
			response->addParameter(CGC_PARAMETER(_T("FromUser"), pUserInfo->getUsername()));
			response->sendResponse();
#ifdef WIN32
			Sleep(5);
#else
			usleep(5000);
#endif
		}
	}

	return 1;
}
