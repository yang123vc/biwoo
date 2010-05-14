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
#include "ConferenceInfo.h"
#include <libCoder/libCoder.h>
#include <sys/timeb.h>

// 1200 = 0x4B0
const short Default_Silence_Value1 = 0x4B0;
const short Default_Silence_Value2 = -0x4B0;

CConferenceInfo::pointer CConferenceInfo::create(CConferInfo::pointer conferInfo, OnConferenceHandler * handler)
{
	return CConferenceInfo::pointer(new CConferenceInfo(conferInfo, handler));
}

CConferenceInfo::CConferenceInfo(CConferInfo::pointer conferInfo, OnConferenceHandler * handler)
: m_conferInfo(conferInfo), m_handler(handler)
, m_currentIndex(0), m_audioMembers(0)
, m_killed(false), m_proca(0), m_procv(0)

{
	BOOST_ASSERT (m_conferInfo.get() != 0);

	m_proca = new boost::thread(boost::bind(&CConferenceInfo::do_proc, this, true));
	m_procv = new boost::thread(boost::bind(&CConferenceInfo::do_proc, this, false));

	m_g729a.g729a_init_encoder();
	m_g729a.g729a_init_decoder();
}
CConferenceInfo::~CConferenceInfo(void)
{
	m_killed = true;
	m_proca->join();
	delete m_proca;
	m_proca = 0;
	m_procv->join();
	delete m_procv;
	m_procv = 0;

	closeConference();
}

int CConferenceInfo::addMember(void * key, CConferenceMember::pointer member)
{
	BOOST_ASSERT (key != 0);
	BOOST_ASSERT (member.get() != 0);

	member->setIndex(++m_currentIndex);

	// ?
	m_members.insert(key, member);
	return m_currentIndex;
}

//bool CConferenceInfo::getMember(void * key, CConferenceMember::pointer & outMember, bool erase)
//{
//	return m_members.find(key, outMember, erase);
//}

void CConferenceInfo::memberClosed(void * key)
{
	CConferenceMember::pointer member;
	if (m_members.find(key, member, true))
	{
		member->setClosed(true);
		if (m_handler)
			m_handler->onMemberClosed(member, false);

		if (m_members.empty())
			m_currentIndex = 0;
	}
}

void CConferenceInfo::closeConference(void)
{
	m_currentIndex = 0;
	CLockMap<void*, CConferenceMember::pointer>::iterator iterMember;
	boost::mutex::scoped_lock lockMember(m_members.mutex());
	for (iterMember=m_members.begin(); iterMember!=m_members.end(); iterMember++)
	{
		CConferenceMember::pointer member = iterMember->second;
		member->setClosed(true);

		if (m_handler)
			m_handler->onMemberClosed(member, true);
	}
	m_members.clear(false);
}

void CConferenceInfo::enableAudioSend(bool enable)
{
	CLockMap<void*, CConferenceMember::pointer>::iterator iterMember;
	boost::mutex::scoped_lock lockMember(m_members.mutex());
	for (iterMember=m_members.begin(); iterMember!=m_members.end(); iterMember++)
	{
		iterMember->second->setAudioSend(enable);
	}
}

void CConferenceInfo::enableAudioRecv(bool enable)
{
	CLockMap<void*, CConferenceMember::pointer>::iterator iterMember;
	boost::mutex::scoped_lock lockMember(m_members.mutex());
	for (iterMember=m_members.begin(); iterMember!=m_members.end(); iterMember++)
	{
		iterMember->second->setAudioRecv(enable);
	}
}

void CConferenceInfo::enableVideoRecv(bool enable)
{
	CLockMap<void*, CConferenceMember::pointer>::iterator iterMember;
	boost::mutex::scoped_lock lockMember(m_members.mutex());
	for (iterMember=m_members.begin(); iterMember!=m_members.end(); iterMember++)
	{
		iterMember->second->setVideoRecv(enable);
	}
}

void CConferenceInfo::enableAudioSend(int memberIndex, bool enable)
{
	CLockMap<void*, CConferenceMember::pointer>::iterator iterMember;
	boost::mutex::scoped_lock lockMember(m_members.mutex());
	for (iterMember=m_members.begin(); iterMember!=m_members.end(); iterMember++)
	{
		CConferenceMember::pointer member = iterMember->second;
		if (member->getIndex() == memberIndex)
		{
			member->setAudioSend(enable);
			return;
		}
	}
}

void CConferenceInfo::enableAudioRecv(int memberIndex, bool enable)
{
	CLockMap<void*, CConferenceMember::pointer>::iterator iterMember;
	boost::mutex::scoped_lock lockMember(m_members.mutex());
	for (iterMember=m_members.begin(); iterMember!=m_members.end(); iterMember++)
	{
		CConferenceMember::pointer member = iterMember->second;
		if (member->getIndex() == memberIndex)
		{
			member->setAudioRecv(enable);
			return;
		}
	}
}

void CConferenceInfo::enableVideoSend(int memberIndex, bool enable)
{
	CLockMap<void*, CConferenceMember::pointer>::iterator iterMember;
	boost::mutex::scoped_lock lockMember(m_members.mutex());
	for (iterMember=m_members.begin(); iterMember!=m_members.end(); iterMember++)
	{
		CConferenceMember::pointer member = iterMember->second;
		if (member->getIndex() == memberIndex)
		{
			member->setVideoSend(enable);
		}else if (enable)
		{
			// 其他要设为 false
			member->setVideoSend(false);
		}
	}
}

void CConferenceInfo::enableVideoRecv(int memberIndex, bool enable)
{
	CLockMap<void*, CConferenceMember::pointer>::iterator iterMember;
	boost::mutex::scoped_lock lockMember(m_members.mutex());
	for (iterMember=m_members.begin(); iterMember!=m_members.end(); iterMember++)
	{
		CConferenceMember::pointer member = iterMember->second;
		if (member->getIndex() == memberIndex)
		{
			member->setVideoRecv(enable);
			return;
		}
	}
}

void CConferenceInfo::enableVideoRecvIndex(int memberIndex, int allowMemberIndex)
{
	CLockMap<void*, CConferenceMember::pointer>::iterator iterMember;
	boost::mutex::scoped_lock lockMember(m_members.mutex());
	for (iterMember=m_members.begin(); iterMember!=m_members.end(); iterMember++)
	{
		CConferenceMember::pointer member = iterMember->second;
		if (member->getIndex() == memberIndex)
		{
			member->setRecvIndex(allowMemberIndex);
			return;
		}
	}
}


void CConferenceInfo::enableVideoRecv(int fromMemberIndex, int toMemberIndex, bool enable)
{
	CConferenceMember::pointer fromMember;
	CConferenceMember::pointer toMember;

	CLockMap<void*, CConferenceMember::pointer>::iterator iterMember;
	boost::mutex::scoped_lock lockMember(m_members.mutex());
	for (iterMember=m_members.begin(); iterMember!=m_members.end(); iterMember++)
	{
		CConferenceMember::pointer member = iterMember->second;
		if (member->getIndex() == fromMemberIndex)
		{
			fromMember = member;
		}
		if (member->getIndex() == fromMemberIndex)
		{
			toMember = member;
		}
		if (fromMember.get() != 0 && toMember.get() != 0)
		{
			break;
		}
	}

	if (enable)
	{
		// 

	}else
	{

	}
}

bool CConferenceInfo::isLimitMaxNumbers(void)
{
	return m_conferInfo->maxNumbers() > 0 && m_members.size() >= m_conferInfo->maxNumbers();
}

void CConferenceInfo::onReceiveEvent(CRTPData::pointer receiveData, const DoRtpHandler * pDoRtpHandler, void * rtpParam)
{
	BOOST_ASSERT (receiveData.get() != NULL);
	BOOST_ASSERT (pDoRtpHandler != NULL);
	BOOST_ASSERT (rtpParam != NULL);

	CConferenceMember * pDataConferenceMember = (CConferenceMember*)rtpParam;
	if (pDataConferenceMember->getClosed() || pDataConferenceMember->getAudioHandler() == 0 || pDataConferenceMember->getVideoHandler() == 0)
		return;

	if (receiveData->size() == 4)
	{
		if (pDataConferenceMember->getAudioHandler() == pDoRtpHandler)
		{
			pDataConferenceMember->getAudioHandler()->doClearDest();
			pDataConferenceMember->getAudioHandler()->doAddDest(receiveData->destip(), receiveData->destport());
		}else if (pDataConferenceMember->getVideoHandler() == pDoRtpHandler)
		{
			pDataConferenceMember->getVideoHandler()->doClearDest();
			pDataConferenceMember->getVideoHandler()->doAddDest(receiveData->destip(), receiveData->destport());
		}
		return;
	}

	if (!pDataConferenceMember->getAudioSend() && pDataConferenceMember->getAudioHandler() == pDoRtpHandler)
	{
		// member audio closed
		return;
	}else if (!pDataConferenceMember->getVideoSend() && pDataConferenceMember->getVideoHandler() == pDoRtpHandler)
	{
		// member video closed
		return;
	}

	bool is_audioData = pDataConferenceMember->getAudioHandler() == pDoRtpHandler;
	if (is_audioData)
	{
		// decode PCM audio

		short v1 = 0;
		short v2 = 0;
		bool has_voice = false;
		switch (receiveData->payloadtype())
		{
		case 8:
			{
				long size = receiveData->size();
				unsigned char * tempBuffer = new unsigned char[size * 2];
				G711Coder::DecodeA((short*)tempBuffer, receiveData->data(), size);
				receiveData->data(tempBuffer, size * 2);

				has_voice = v1 > Default_Silence_Value1 || v2 < Default_Silence_Value2;
			}break;
		case 0:
			{
				long size = receiveData->size();
				unsigned char * tempBuffer = new unsigned char[size * 2];
				G711Coder::DecodeU((short*)tempBuffer, receiveData->data(), size, &v1, &v2);
				receiveData->data(tempBuffer, size * 2);

				has_voice = v1 > Default_Silence_Value1 || v2 < Default_Silence_Value2;
				//if (v1 > Default_Silence_Value1 || v2 < Default_Silence_Value2)
				//{
				//	int i = 0;
				//}
			}break;
		case 18:
			{
				long size = receiveData->size();
				unsigned char * tempBuffer = new unsigned char[(int)(size * (80.0/10.0)*2)];
				m_g729a.g729a_decoder(receiveData->data(), size, (short*)tempBuffer, &v1, &v2);
				receiveData->data(tempBuffer, (long)(size*(80.0/10.0)*2));

				has_voice = v1 > Default_Silence_Value1 || v2 < Default_Silence_Value2;
			}break;
		default:
			break;
		}
		pDataConferenceMember->setRecvDataTime();
		m_datasa.add(CMemberData::create(receiveData, pDoRtpHandler, rtpParam, !has_voice));
	}else
	{
		m_datasv.add(CMemberData::create(receiveData, pDoRtpHandler, rtpParam));
	}

}

void CConferenceInfo::encode_audio(unsigned short payloadtype, CBuffer::pointer inBuffer, unsigned char ** outBuffer, long & outSize)
{
	BOOST_ASSERT (inBuffer.get() != NULL && inBuffer->size() > 2);

	// re-encode audio
	switch (payloadtype)
	{
	case 8:
		{
			outSize = inBuffer->size()/2;
			*outBuffer = new unsigned char[outSize];
			G711Coder::EncodeA(*outBuffer,(const short*)inBuffer->buffer(), outSize);
		}break;
	case 0:
		{
			outSize = inBuffer->size()/2;
			*outBuffer = new unsigned char[outSize];
			G711Coder::EncodeU(*outBuffer,(const short*)inBuffer->buffer(), outSize);
		}break;
	case 18:
		{
			outSize = (long)(inBuffer->size()*(10.0/80.0)/2);
			*outBuffer = new unsigned char[(int)(inBuffer->size()*(10.0/80.0))];
			m_g729a.g729a_encoder((const short*)inBuffer->buffer(), inBuffer->size()/2, *outBuffer);
		}break;
	default:
		break;
	}
}

bool CConferenceInfo::mix_member_frame(CConferenceMember::pointer member, const CLockMap<CConferenceMember*, CMemberData::pointer> & audios, int & outTimestamp)
{
	BOOST_ASSERT (member.get() != 0);

	if (member->getClosed()) return false;

	bool result = false;
	bool first = true;
	boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(audios.mutex()));
	CLockMap<CConferenceMember*, CMemberData::pointer>::const_iterator iter;
	for (iter=audios.begin(); iter!= audios.end(); iter++)
	{
		if (iter->second->getRtpParam() == member.get())
			continue;

		if (iter->second->isSilence())
			continue;

		result = true;
		member->audioBuffer()->remalloc(iter->second->getRtpData()->size());

		if (first)
		{
			first = false;
			member->audioBuffer()->set(iter->second->getRtpData()->data(), iter->second->getRtpData()->size());
		}else
		{
			mix_slinear_frames((char*)const_cast<void*>(member->audioBuffer()->buffer()), (const char*)iter->second->getRtpData()->data(), iter->second->getRtpData()->size()/2);
		}

		if (iter->second->getRtpData()->timestamp() > (unsigned int)outTimestamp)
			outTimestamp = iter->second->getRtpData()->timestamp();
	}

	return result;
}

void CConferenceInfo::mix_slinear_frames( char *dst, const char *src, int samples )
{
	if ( dst == NULL ) return ;
	if ( src == NULL ) return ;

	int i, val ;

	for ( i = 0 ; i < samples ; ++i )
	{
		val = ( (short*)dst )[i] + ( (short*)src )[i] ;

		if ( val > 0x7fff )
		{
			( (short*)dst )[i] = 0x7fff - 1 ;
		}
		else if ( val < -0x7fff )
		{
			( (short*)dst )[i] = -0x7fff + 1 ;
		}else
		{
			( (short*)dst )[i] = val ;
		}
	}
}

int CConferenceInfo::countAudioMember(void)
{
	struct timeb tNow;
	ftime(&tNow);

	m_audioMembers = 0;
	boost::mutex::scoped_lock lock(m_members.mutex());
	CLockMap<void*, CConferenceMember::pointer>::iterator iter;
	for (iter=m_members.begin(); iter!= m_members.end(); iter++)
	{
		//if (!iter->second->getClosed() && iter->second->getAudioCounter() > 0)
		if (!iter->second->getClosed() && iter->second->hasRecvData(tNow))
			m_audioMembers++;
	}
	return m_audioMembers;
}

void CConferenceInfo::setToMemberFront(CLockList<CMemberData::pointer> & destList, CMemberData::pointer memberData) const
{
	BOOST_ASSERT (memberData.get() != 0);

	CLockList<CMemberData::pointer>::iterator iter;
	for (iter=destList.begin(); iter!= destList.end(); iter++)
	{
		//CMemberData::pointer memberDataTemp = *iter;
		if ((*iter)->getRtpParam() == memberData->getRtpParam())
		{
			destList.insert(iter, memberData);
			return;
		}
	}

	destList.add(memberData);
}

void CConferenceInfo::sendAudioFrame(const CLockMap<CConferenceMember*, CMemberData::pointer> & audios)
{
	if (audios.empty()) return;

	boost::mutex::scoped_lock lock(m_members.mutex());
	CLockMap<void*, CConferenceMember::pointer>::iterator iter;
	for (iter=m_members.begin(); iter!= m_members.end(); iter++)
	{
		// member don't receive audio
		if (!iter->second->getAudioRecv())
			continue;

		int timestamp = 0;
		if (mix_member_frame(iter->second, audios, timestamp))
		{
			DoRtpHandler * pDoRtpHandler = iter->second->getAudioHandler();

			long sendSize = 0;
			unsigned char * tempBuffer = 0;
			encode_audio(pDoRtpHandler->doGetMediaType(), iter->second->audioBuffer(), &tempBuffer, sendSize);

			//int timestamp = 0;			
			pDoRtpHandler->doSendData(tempBuffer, sendSize, timestamp);
			delete[] tempBuffer;
		}
	}

}

void CConferenceInfo::sendVideoFrame(CMemberData::pointer memberData)
{
	BOOST_ASSERT (memberData.get() != 0);

	boost::mutex::scoped_lock lock(m_members.mutex());
	CConferenceMember * pDataConferenceMember = (CConferenceMember*)memberData->getRtpParam();
	if (pDataConferenceMember->getClosed()) return;

	BOOST_ASSERT (pDataConferenceMember->getVideoHandler() == memberData->getDoRtpHandler());
	CLockMap<void*, CConferenceMember::pointer>::iterator iter;
	for (iter=m_members.begin(); iter!= m_members.end(); iter++)
	{
		// don't send to owner
		//if (pDataConferenceMember == iter->second.get())
		//	continue;

		// member don't receive video
		if (!iter->second->getVideoRecv())
			continue;

		// The user don't receive this member video
		if (iter->second->getRecvIndex() != pDataConferenceMember->getIndex()) 
		{
			continue;
		}

		int timestamp = memberData->getRtpData()->timestamp();
		//int timestamp = 0;
		DoRtpHandler * pDoRtpHandler = iter->second->getVideoHandler();
		pDoRtpHandler->doSendData(memberData->getRtpData()->data(), memberData->getRtpData()->size(), timestamp);
	}
}

void CConferenceInfo::doProc(bool audio_proc)
{
	if (audio_proc)
		doAudioProc();
	else
		doVideoProc();
}

void CConferenceInfo::doAudioProc(void)
{
	bool bGetTempMember = false;
	CLockMap<CConferenceMember*, CMemberData::pointer> toSendMembers;
	CLockList<CMemberData::pointer> memberTemps;

	while (!m_killed)
	{
		CMemberData::pointer memberData;
		if (bGetTempMember)
		{
			if (!memberTemps.front(memberData, true))
			{
				bGetTempMember = false;
				continue;
			}

		}else if (!m_datasa.front(memberData, true))
		{
#ifdef WIN32
			Sleep(20);
#else
			usleep(20000);
#endif
			continue;
		}

		CConferenceMember * pDataConferenceMember = (CConferenceMember*)memberData->getRtpParam();
		if (pDataConferenceMember->getClosed() || pDataConferenceMember->getAudioHandler() == 0 || pDataConferenceMember->getVideoHandler() == 0)
		{
			continue;
		}

		BOOST_ASSERT (pDataConferenceMember->getAudioHandler() == memberData->getDoRtpHandler());

		if (toSendMembers.exist(pDataConferenceMember))
		{
			if (toSendMembers.size() < (unsigned int)countAudioMember())
			{
				setToMemberFront(memberTemps, memberData);
				bGetTempMember = false;
				continue;
			}

			sendAudioFrame(toSendMembers);
			toSendMembers.clear(false);
		}

		toSendMembers.insert(pDataConferenceMember, memberData);
		bGetTempMember = !memberTemps.empty();
	}

}

void CConferenceInfo::doVideoProc(void)
{
	while (!m_killed)
	{
		CMemberData::pointer memberData;
		if (!m_datasv.front(memberData, true))
		{
#ifdef WIN32
			Sleep(20);
#else
			usleep(20000);
#endif
			continue;
		}

		CConferenceMember * pDataConferenceMember = (CConferenceMember*)memberData->getRtpParam();
		if (pDataConferenceMember->getClosed() || pDataConferenceMember->getAudioHandler() == 0 || pDataConferenceMember->getVideoHandler() == 0)
		{
			continue;
		}

		BOOST_ASSERT (pDataConferenceMember->getVideoHandler() == memberData->getDoRtpHandler());

		sendVideoFrame(memberData);
	}

}

void CConferenceInfo::do_proc(CConferenceInfo * owner, bool audio_proc)
{
	BOOST_ASSERT (owner != 0);
	owner->doProc(audio_proc);
}

