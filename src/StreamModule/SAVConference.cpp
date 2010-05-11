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

#include <CGCBase/includeapp.h>
using namespace cgc;

#include "AVSProxy.h"

//////////////////////////
extern "C" int CGC_API CreateConference(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	const tstring & conferenceName = request->getParameterValue(_T("Conference"), _T(""));
	unsigned short maxNumbers = request->getParameterValue(_T("MaxNumbers"), 0);

	CConferInfo::pointer conferInfo = CConferInfo::create(conferenceName, maxNumbers);
	gAVSProxy.m_conference.addConference(conferInfo);

	gApplication->log(DL_INFO, _T("Create Conference %s"), conferenceName.c_str());
	return 0;
}

extern "C" int CGC_API EnableAudioSend(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	const tstring & conferenceName = request->getParameterValue(_T("Conference"), _T(""));
	int memberIndex = request->getParameterValue(_T("Index"), 0);
	bool enable = request->getParameterValue(_T("Enable"), 0) == 1;

	if (memberIndex == 0)
		gAVSProxy.m_conference.enableAudioSend(conferenceName, enable);
	else
		gAVSProxy.m_conference.enableAudioSend(conferenceName, memberIndex, enable);

	return 0;
}

extern "C" int CGC_API EnableAudioRecv(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	const tstring & conferenceName = request->getParameterValue(_T("Conference"), _T(""));
	int memberIndex = request->getParameterValue(_T("Index"), 0);
	bool enable = request->getParameterValue(_T("Enable"), 0) == 1;

	if (memberIndex == 0)
		gAVSProxy.m_conference.enableAudioRecv(conferenceName, enable);
	else
		gAVSProxy.m_conference.enableAudioRecv(conferenceName, memberIndex, enable);

	return 0;
}

extern "C" int CGC_API EnableVideoSend(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	const tstring & conferenceName = request->getParameterValue(_T("Conference"), _T(""));
	int memberIndex = request->getParameterValue(_T("Index"), 0);
	bool enable = request->getParameterValue(_T("Enable"), 0) == 1;

	gAVSProxy.m_conference.enableVideoSend(conferenceName, memberIndex, enable);
	return 0;
}

extern "C" int CGC_API EnableVideoRecv(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	const tstring & conferenceName = request->getParameterValue(_T("Conference"), _T(""));
	int memberIndex = request->getParameterValue(_T("Index"), 0);
	bool enable = request->getParameterValue(_T("Enable"), 0) == 1;

	if (memberIndex == 0)
		gAVSProxy.m_conference.enableVideoRecv(conferenceName, enable);
	else
		gAVSProxy.m_conference.enableVideoRecv(conferenceName, memberIndex, enable);

	return 0;
}

extern "C" int CGC_API SelectVideoRecv(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	const tstring & conferenceName = request->getParameterValue(_T("Conference"), _T(""));
	int memberIndex = request->getParameterValue(_T("SrcIndex"), 0);
	int selectIndex = request->getParameterValue(_T("SelectIndex"), 0);

	//if (memberIndex == 0)
	//	gAVSProxy.m_conference.enableVideoRecv(conferenceName, enable);
	//else
	//	gAVSProxy.m_conference.enableVideoRecv(conferenceName, memberIndex, enable);
	gAVSProxy.m_conference.enableVideoRecvIndex(conferenceName,memberIndex,selectIndex);
	return 0;
}

extern "C" int CGC_API SelectVideoRecvByIndentify(const cgcRequest::pointer & request, cgcResponse::pointer response, cgcSession::pointer session)
{
	const tstring & conferenceName = request->getParameterValue(_T("Conference"), _T(""));
	const tstring & MemberIndentify = request->getParameterValue(_T("MemberIndentify"),_T(""));
	int selectIndex = request->getParameterValue(_T("SelectIndex"), 0);

	//if (memberIndex == 0)
	//	gAVSProxy.m_conference.enableVideoRecv(conferenceName, enable);
	//else
	//	gAVSProxy.m_conference.enableVideoRecv(conferenceName, memberIndex, enable);
	gAVSProxy.m_conference.enableVideoRecvIndexByIndentify(conferenceName,MemberIndentify,selectIndex);
	return 0;
}
