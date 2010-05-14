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

// ConferenceInfo.h file here
#ifndef __ConferenceInfo_h__
#define __ConferenceInfo_h__

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <stl/lockmap.h>
#include <stl/locklist.h>
#include <stl/buffer.h>
#include "../../ThirdParty/libRTP/libRTP.h"
#include "../../ThirdParty/libG729a/libG729a.h"
#include "ConferenceMember.h"
#include "MemberData.h"
#include "conferinfo.h"

class OnConferenceHandler
{
public:
	virtual void onMemberClosed(CConferenceMember::pointer member, bool conferenceClosed) = 0;
};

class CConferenceInfo
	: public OnRtpHandler
{
public:
	typedef boost::shared_ptr<CConferenceInfo> pointer;
	static CConferenceInfo::pointer create(CConferInfo::pointer conferInfo, OnConferenceHandler * handler);

	const tstring & getname(void) const {return m_conferInfo->name();}
	CConferInfo::pointer conferInfo(void) const {return m_conferInfo;}
	void doProc(bool audio_proc);

	int addMember(void * key, CConferenceMember::pointer member);
	//bool getMember(void * key, CConferenceMember::pointer & outMember, bool erase);
	void memberClosed(void * key);
	void closeConference(void);
	size_t memberSize(void) const {return m_members.size();}
	bool emptyMember(void) const {return m_members.empty();}

	void enableAudioSend(bool enable);
	void enableAudioRecv(bool enable);
	void enableVideoRecv(bool enable);
	void enableAudioSend(int memberIndex, bool enable);
	void enableAudioRecv(int memberIndex, bool enable);
	void enableVideoSend(int memberIndex, bool enable);
	void enableVideoRecv(int memberIndex, bool enable);
	void enableVideoRecv(int fromMemberIndex, int toMemberIndex, bool enable);

	void enableVideoRecvIndex(int memberIndex, int allowMemberIndex); ///add by xap


	bool isLimitMaxNumbers(void);

private:
	// OnRtpHandler handler
	virtual void onReceiveEvent(CRTPData::pointer receiveData, const DoRtpHandler * pDoRtpHandler, void * param);

	void sendAudioFrame(const CLockMap<CConferenceMember*, CMemberData::pointer> & audios);
	void sendVideoFrame(CMemberData::pointer memberData);

	static void do_proc(CConferenceInfo * owner, bool audio_proc);

	void encode_audio(unsigned short payloadtype, CBuffer::pointer inBuffer, unsigned char ** outBuffer, long & outSize);

	static bool mix_member_frame(CConferenceMember::pointer member, const CLockMap<CConferenceMember*, CMemberData::pointer> & audios, int & outTimestamp);
	static void mix_slinear_frames( char *dst, const char *src, int samples );

	int countAudioMember(void);
	void setToMemberFront(CLockList<CMemberData::pointer> & destList, CMemberData::pointer memberData) const;

	void doAudioProc(void);
	void doVideoProc(void);

public:
	CConferenceInfo(CConferInfo::pointer conferInfo, OnConferenceHandler * handler);
	~CConferenceInfo(void);

private:
	CConferInfo::pointer m_conferInfo;
	OnConferenceHandler * m_handler;
	CLockMap<void*, CConferenceMember::pointer> m_members;
	int m_currentIndex;
	int m_audioMembers;

	bool m_killed;
	//CLockListPtr<boost::thread*> m_procs;
	boost::thread * m_proca;
	boost::thread * m_procv;
	CLockList<CMemberData::pointer> m_datasa;
	CLockList<CMemberData::pointer> m_datasv;

	// coder
	CG729a m_g729a;

};

#endif // __ConferenceInfo_h__

