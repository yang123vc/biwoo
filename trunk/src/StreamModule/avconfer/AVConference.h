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

// AVConference.h file here
#ifndef __AVConference_h__
#define __AVConference_h__

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <stl/lockmap.h>
#include <libSIP/libSIP.h>
#include <libRTP/libRTP.h>
#include "ConferenceInfo.h"

#ifdef WIN32
#ifdef _DEBUG
//#pragma comment(lib,"msvcrtd.lib")
#pragma comment(linker,"/NODEFAULTLIB:libcmtd.lib") 
#else
//#pragma comment(lib,"msvcrt.lib")	// ??Release编译时，不同切换注释可以播放视频
#pragma comment(linker,"/NODEFAULTLIB:libcmt.lib") 
#endif
#endif

struct UserInfo
{
	tstring conferenceName;
	tstring memberIndentify;
	int Index;
};

class CAVConference
	: public OnSipHandler
	, public OnConferenceHandler
{
public:
	bool initsip(const CSipParameter & sipp);
	void quitsip(void);

	CConferenceInfo::pointer getConference(const tstring & conferenceName);
	CConferenceInfo::pointer addConference(CConferInfo::pointer conferInfo);

	void enableAudioSend(const tstring & conferenceName, bool enable);
	void enableAudioRecv(const tstring & conferenceName, bool enable);
	void enableVideoRecv(const tstring & conferenceName, bool enable);
	void enableAudioSend(const tstring & conferenceName, int memberIndex, bool enable);
	void enableAudioRecv(const tstring & conferenceName, int memberIndex, bool enable);
	void enableVideoSend(const tstring & conferenceName, int memberIndex, bool enable);
	void enableVideoRecv(const tstring & conferenceName, int memberIndex, bool enable);

	void enableVideoRecvIndex(const tstring & conferenceName, int memberIndex, int allowMemberIndex); //add by xap
	void enableVideoRecvIndexByIndentify(const tstring & conferenceName,const tstring & MemberIndentify,int selectIndex); //add by xap
protected:
	int getIndexByUserIndentify(const tstring & conferenceName,const tstring & MemberIndentify); //add by xap
	int deleteUserInfoByUserIndentify(const tstring & conferenceName,const tstring & MemberIndentify); //add by xap

private:
	// OnSipHandler handler
	virtual void onSipEvent(SipEventInfo::pointer eventInfo);

	// OnConferenceHandler handler
	virtual void onMemberClosed(CConferenceMember::pointer member, bool conferenceClosed);

public:
	CAVConference(void);
	~CAVConference(void);

private:
	CRtp m_rtp;
	unsigned short m_currentRtpPort;

	// SIP
	CSip m_sip;
	DoSipHandler::pointer m_sipHandler;

	CLockMap<tstring, CConferenceInfo::pointer> m_conferences;
	CLockMap<void*, CConferenceInfo::pointer> m_conferences2;	// SipCallInfo->
	CLockList<CConferenceMember::pointer> m_clearMember;

	CLockList<UserInfo> m_MemberInfo;//add by xap
};


#endif // __AVConference_h__

