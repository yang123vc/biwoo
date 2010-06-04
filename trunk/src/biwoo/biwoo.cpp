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

// biwoo.cpp : Defines the initialization routines for the DLL.
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

#include <boost/filesystem.hpp>
// cgc head file
#include <CGCBase/includeapp.h>
#include <CGCBase/cgcBodb.h>
#include <CGCBase/cgcString.h>
using namespace cgc;

#include "sendresponse.h"

///////////////////////////////
// 
#include "AVSProxy.h"
#include "xmlparseusers.h"

cgcBodb::pointer gBodbService;
CBodbHandler::pointer gBodbHandler;

extern "C" bool CGC_API CGC_Module_Init(void)
{
	cgcBodb::pointer bodbService = CGC_BODBSERVICE_DEF(gCgcService->getService("BodbService"));
	BOOST_ASSERT (bodbService.get() != NULL);

	tstring sBodbPath(gApplication->getAppConfPath());
	sBodbPath.append(_T("/db"));
	CBodbHandler::pointer bodbHandler = bodbService->bodb_init(sBodbPath.c_str());
	if (bodbHandler.get() == NULL)
	{
		bodbService.reset();
		return false;
	}

	gAVSProxy = CAVSProxy::create(bodbService, bodbHandler);
	if (!gAVSProxy->load())
	{
		gAVSProxy.reset();
		bodbService->bodb_exit(bodbHandler);
		bodbService.reset();
		return false;
	}

	// Load datas from bodb.
	//gAVSProxy->setpath(sBodbPath);
	//if (!gAVSProxy->load())
	//{
	//	return false;
	//}

	namespace fs = boost::filesystem;
	std::string filePath(gApplication->getAppConfPath());
	filePath.append("/File");
	fs::path pathModuleFile(filePath, fs::native);
	if (!boost::filesystem::exists(pathModuleFile))
	{
		fs::create_directory(pathModuleFile);
	}
	// Load the setting userinfo.
	/*
	tstring sXmlPath(gApplication->getAppConfPath());
	sXmlPath.append(_T("/default_users.xml"));
	xmlparseusers parseusers;
	parseusers.load(sXmlPath);
	CLockMap<tstring, CUserInfo::pointer>::iterator iter;
	for (iter=parseusers.m_users.begin(); iter!=parseusers.m_users.end(); iter++)
	{
		if (!gApplication->existAttribute(BMT_ALLUSERS, iter->second->getAccount()))
		//if (!gAVSProxy->m_users.exist(iter->second->getAccount()))
		{
			//gAVSProxy->m_users.insert(iter->second->getAccount(), iter->second);
			gAVSProxy->addUserinfo(iter->second);
		}
	}
	parseusers.m_users.clear();
	*/

	return true;
}

extern "C" void CGC_API CGC_Module_Free(void)
{
	StringPointerMapPointer stringPointerMapPointer = gApplication->getStringAttributes(BMT_ACCOUNTIDS, false);
	if (stringPointerMapPointer.get() != NULL)
	{
		boost::mutex::scoped_lock lock(stringPointerMapPointer->mutex());
		CPointerMap<tstring>::iterator iter;
		for (iter=stringPointerMapPointer->begin(); iter!=stringPointerMapPointer->end(); iter++)
		{
			CAccountInfo::pointer accountInfo = (const CAccountInfo::pointer&)iter->second;

			sendUserOnOfflineEvent(accountInfo, accountInfo->getUserinfo(), false, true);
#ifdef WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
		}
	}

	gAVSProxy->close();
	gAVSProxy.reset();

	gApplication->clearAllAtrributes();
}

extern "C" int CGC_API LoadSetting(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	// Request

	response->setParameter(gApplication->getInitParameter("FILESERVER"));
	response->sendResponse();

	return 0;
}
