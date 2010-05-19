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

#ifdef WIN32
#include "windows.h"
#endif
#include "AVConference.h"

CAVConference::CAVConference(void)
: m_currentRtpPort(9010)

{

}

CAVConference::~CAVConference(void)
{
	quitsip();
}

bool CAVConference::initsip(const CSipParameter & sipp)
{
	/*tstring sUA = _T("yhz");
	tstring sPwd = _T("yhz");
	tstring sIdentity = _T("sip:yhz@192.168.19.77");
	tstring sProxy = _T("sip:192.168.19.84:5060");

	CSipParameter sipp;
	sipp.ua(sUA);
	sipp.pwd(sPwd);
	sipp.identity(sIdentity);
	sipp.proxy(sProxy);
	sipp.sipport(sipport);*/

	bool ret = m_sip.initSip(sipp, this);
	return ret;
}

void CAVConference::quitsip(void)
{
	m_sip.quitSip();

	CLockMap<tstring, CConferenceInfo::pointer>::iterator iterConference;
	boost::mutex::scoped_lock lockConference(m_conferences.mutex());
	for (iterConference=m_conferences.begin(); iterConference!= m_conferences.end(); iterConference++)
	{
		CConferenceInfo::pointer conferenceInfo = iterConference->second;
		conferenceInfo->closeConference();
	}
	m_conferences.clear(false);
	m_clearMember.clear();
	m_conferences2.clear();
}

CConferenceInfo::pointer CAVConference::getConference(const tstring & conferenceName)
{
	CConferenceInfo::pointer conferenceInfo;
	m_conferences.find(conferenceName, conferenceInfo, false);
	return conferenceInfo;
}

CConferenceInfo::pointer CAVConference::addConference(CConferInfo::pointer conferInfo)
{
	BOOST_ASSERT (conferInfo.get() != 0);

	CConferenceInfo::pointer conferenceInfo;
	if (!m_conferences.find(conferInfo->name(), conferenceInfo, false))
	{
		conferenceInfo = CConferenceInfo::create(conferInfo, (OnConferenceHandler*)this);
		m_conferences.insert(conferInfo->name(), conferenceInfo);
	}
	return conferenceInfo;
}

void CAVConference::enableAudioSend(const tstring & conferenceName, bool enable)
{
	CConferenceInfo::pointer conferenceInfo;
	if (m_conferences.find(conferenceName, conferenceInfo, false))
	{
		conferenceInfo->enableAudioSend(enable);
	}
}

void CAVConference::enableAudioRecv(const tstring & conferenceName, bool enable)
{
	CConferenceInfo::pointer conferenceInfo;
	if (m_conferences.find(conferenceName, conferenceInfo, false))
	{
		conferenceInfo->enableAudioRecv(enable);
	}
}

void CAVConference::enableVideoRecv(const tstring & conferenceName, bool enable)
{
	CConferenceInfo::pointer conferenceInfo;
	if (m_conferences.find(conferenceName, conferenceInfo, false))
	{
		conferenceInfo->enableVideoRecv(enable);
	}
}

void CAVConference::enableAudioSend(const tstring & conferenceName, int memberIndex, bool enable)
{
	CConferenceInfo::pointer conferenceInfo;
	if (m_conferences.find(conferenceName, conferenceInfo, false))
	{
		conferenceInfo->enableAudioSend(memberIndex, enable);
	}
}

void CAVConference::enableAudioRecv(const tstring & conferenceName, int memberIndex, bool enable)
{
	CConferenceInfo::pointer conferenceInfo;
	if (m_conferences.find(conferenceName, conferenceInfo, false))
	{
		conferenceInfo->enableAudioRecv(memberIndex, enable);
	}
}

void CAVConference::enableVideoSend(const tstring & conferenceName, int memberIndex, bool enable)
{
	CConferenceInfo::pointer conferenceInfo;
	if (m_conferences.find(conferenceName, conferenceInfo, false))
	{
		conferenceInfo->enableVideoSend(memberIndex, enable);
	}
}

void CAVConference::enableVideoRecv(const tstring & conferenceName, int memberIndex, bool enable)
{
	CConferenceInfo::pointer conferenceInfo;
	if (m_conferences.find(conferenceName, conferenceInfo, false))
	{
		conferenceInfo->enableVideoRecv(memberIndex, enable);
	}
}

void CAVConference::onSipEvent(SipEventInfo::pointer eventInfo)
{
	BOOST_ASSERT (eventInfo.get() != 0);

	switch (eventInfo->getEventType())
	{
	case SipEventInfo::ET_Register:
		{
			//if (eventInfo->getEventData() == 2)
			//	m_bIsRegistered = true;
		}break;
	case SipEventInfo::ET_CallInvite:
		{
			BOOST_ASSERT (eventInfo->getCallInfo().get() != NULL);

			tstring conferenceName = eventInfo->getCallInfo()->callee();
			//CConferenceInfo::pointer conferenceInfo = addConference(conferenceName);
			//BOOST_ASSERT (conferenceInfo.get() != NULL);

			if (getIndexByUserIndentify(conferenceName,eventInfo->getCallInfo()->caller())!= -1) ///用户已经存在 add by xap
			{
				m_sip.CallTerminate(eventInfo->getCallInfo());
				return;
			}

			CConferenceInfo::pointer conferenceInfo = getConference(conferenceName);
			if (conferenceInfo.get() == NULL || conferenceInfo->isLimitMaxNumbers())
			{
				// 找不到会议号，或者超过最大会议成员数，直接挂机
				m_sip.CallTerminate(eventInfo->getCallInfo());
				return;
			}

			DoRtpHandler::pointer audioRtpHandler;
			int i=0;
			while(i++ != 20)
			{
				m_currentRtpPort += i*2;
				audioRtpHandler = m_rtp.startRtp(m_currentRtpPort);
				if (audioRtpHandler.get() != NULL)
					break;
			}
			DoRtpHandler::pointer videoRtpHandler;
			i=0;
			while(i++ != 20)
			{
				m_currentRtpPort += i*2;
				videoRtpHandler = m_rtp.startRtp(m_currentRtpPort);
				if (videoRtpHandler.get() != NULL)
					break;
			}

			if (audioRtpHandler.get() && videoRtpHandler.get())
			{
				CConferenceMember::pointer conferenceMember = CConferenceMember::create(audioRtpHandler, videoRtpHandler);
				audioRtpHandler->doSetMediaType(0);		// PCMU
				videoRtpHandler->doSetMediaType(99);	// MPEG4
				audioRtpHandler->doSetRtpHandler(conferenceInfo.get(), conferenceMember.get());
				videoRtpHandler->doSetRtpHandler(conferenceInfo.get(), conferenceMember.get());

				audioRtpHandler->doAddDest(eventInfo->getCallInfo()->remoteIp().c_str(), eventInfo->getCallInfo()->audioPort());
				videoRtpHandler->doAddDest(eventInfo->getCallInfo()->remoteIp().c_str(), eventInfo->getCallInfo()->videoPort());

				m_sip.CallAnswer(eventInfo->getCallInfo(), audioRtpHandler->doGetRtpPort(), videoRtpHandler->doGetRtpPort());

				conferenceMember->setIp(eventInfo->getCallInfo()->remoteIp());
				conferenceMember->setAudioPort(eventInfo->getCallInfo()->audioPort());
				conferenceMember->setVideoPort(eventInfo->getCallInfo()->videoPort());

				//conferenceInfo->addMember(eventInfo->getCallInfo().get(), conferenceMember);
				////add by xap
				int id = conferenceInfo->addMember(eventInfo->getCallInfo().get(), conferenceMember);
				conferenceInfo->enableVideoRecvIndex(id,-1);
				UserInfo curruserInfo;
				curruserInfo.conferenceName = eventInfo->getCallInfo()->callee();
				curruserInfo.memberIndentify = eventInfo->getCallInfo()->caller();
				curruserInfo.Index = id;
				m_MemberInfo.add(curruserInfo);
				////////////
				if (conferenceInfo->memberSize() == 1)
				{
					conferenceMember->setAudioSend(true);
					conferenceMember->setVideoSend(true);
					conferenceMember->setVideoRecv(true);
				}else
				{
					//conferenceMember->setAudioSend(false);
					conferenceMember->setVideoRecv(true);
				}
				m_conferences2.insert(eventInfo->getCallInfo().get(), conferenceInfo);
			}else
			{
				m_rtp.stopRtp(audioRtpHandler);
				m_rtp.stopRtp(videoRtpHandler);
				m_sip.CallTerminate(eventInfo->getCallInfo());
			}


		}break;
	case SipEventInfo::ET_CallAnswered:
		{
			BOOST_ASSERT (eventInfo->getCallInfo().get() != NULL);

			//m_currentCallInfo = eventInfo->getCallInfo();
			//addCallInfoIpPort(eventInfo->getCallInfo());
		}break;
	case SipEventInfo::ET_CallClosed:
		{
			BOOST_ASSERT (eventInfo->getCallInfo().get() != NULL);

			CConferenceInfo::pointer conferenceInfo;
			if (m_conferences2.find(eventInfo->getCallInfo().get(), conferenceInfo, true))
			{
				conferenceInfo->memberClosed(eventInfo->getCallInfo().get());

				// 已经没有成员，删除会议
				deleteUserInfoByUserIndentify(eventInfo->getCallInfo()->callee(),eventInfo->getCallInfo()->caller()); //add by xap

				if (conferenceInfo->emptyMember())
				{
					//m_conferences.remove(conferenceInfo->getname());
				}
			}

			//if (eventInfo->getCallInfo().get() == m_currentCallInfo.get())
			//	m_currentCallInfo.reset();
		}break;
	case SipEventInfo::ET_CallMessageNew:
		{
			BOOST_ASSERT (eventInfo->getCallInfo().get() != NULL);

			switch (eventInfo->getCallInfo()->messageType())
			{
			case SipCallInfo::MT_Dtmf:
				{
					//char dtmf = eventInfo->getCallInfo()->callMessage().c_str()[0];
					CConferenceInfo::pointer conferenceInfo;
					if (m_conferences2.find(eventInfo->getCallInfo().get(), conferenceInfo, false))
					{

					}

				}break;
			default:
				break;
			}
		}break;
	default:
		break;
	}

	//if (m_pSipHandler)
	//	m_pSipHandler->onSipEvent(eventInfo);
}

void CAVConference::onMemberClosed(CConferenceMember::pointer member, bool conferenceClosed)
{
	BOOST_ASSERT (member.get() != 0);

	if (!conferenceClosed)
	{
#ifdef WIN32
		Sleep(100);
#else
		usleep(100000);
#endif
		m_clearMember.add(member);
	}

	m_rtp.stopRtp(member->getAudioHandler());
	m_rtp.stopRtp(member->getVideoHandler());
}


void CAVConference::enableVideoRecvIndex(const tstring & conferenceName, int memberIndex, int allowMemberIndex)
{
	CConferenceInfo::pointer conferenceInfo;
	if (m_conferences.find(conferenceName, conferenceInfo, false))
	{
		conferenceInfo->enableVideoRecvIndex(memberIndex, allowMemberIndex);
	}
}

int CAVConference::getIndexByUserIndentify(const tstring & conferenceName,const tstring & MemberIndentify)
{
	CConferenceInfo::pointer conferenceInfo;
	if (m_conferences.find(conferenceName, conferenceInfo, false))
	{
		CLockList<UserInfo>::iterator iterMember;
		for (iterMember=m_MemberInfo.begin(); iterMember!=m_MemberInfo.end(); iterMember++)
		{
			UserInfo member = *iterMember;
			if ((member.conferenceName == conferenceName)&&(member.memberIndentify == MemberIndentify))
			{
				//member->setRecvIndex(allowMemberIndex);
				return member.Index;
			}
		}	
	}
	return -1;
}

int CAVConference::deleteUserInfoByUserIndentify(const tstring & conferenceName,const tstring & MemberIndentify)
{
	CConferenceInfo::pointer conferenceInfo;
	if (m_conferences.find(conferenceName, conferenceInfo, false))
	{
		CLockList<UserInfo>::iterator iterMember;
		for (iterMember=m_MemberInfo.begin(); iterMember!=m_MemberInfo.end(); iterMember++)
		{
			UserInfo member = *iterMember;
			if ((member.conferenceName == conferenceName)&&(member.memberIndentify == MemberIndentify))
			{
				//member->setRecvIndex(allowMemberIndex);
				//return member.Index;
				m_MemberInfo.erase(iterMember);
				return 1;
			}
		}	
	}
	return -1;
}

void CAVConference::enableVideoRecvIndexByIndentify(const tstring & conferenceName,const tstring & MemberIndentify,int selectIndex)
{
	CConferenceInfo::pointer conferenceInfo;
	if (m_conferences.find(conferenceName, conferenceInfo, false))
	{
		conferenceInfo->enableVideoRecvIndex(getIndexByUserIndentify(conferenceName,MemberIndentify), selectIndex);
	}
}
