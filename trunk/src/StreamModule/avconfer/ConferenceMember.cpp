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

#include "ConferenceMember.h"

CConferenceMember::pointer CConferenceMember::create(DoRtpHandler::pointer audio, DoRtpHandler::pointer video)
{
	return CConferenceMember::pointer(new CConferenceMember(audio, video));
}

CConferenceMember::CConferenceMember(DoRtpHandler::pointer audio, DoRtpHandler::pointer video)
: m_audioHandler(audio), m_videoHandler(video)
, m_ip(_T("")), m_audioPort(0), m_videoPort(0)
, m_audioSend(true), m_audioRecv(true), m_videoSend(false), m_videoRecv(false)
, m_bClosed(false), m_index(0),m_recvIndex(0)

{
	memset(&m_tLastRecvTime, 0, sizeof(m_tLastRecvTime));
	m_audioBuffer = CBuffer::create();
}

void CConferenceMember::setRecvDataTime(void)
{
	ftime(&m_tLastRecvTime);
}

bool CConferenceMember::hasRecvData(struct timeb & now)
{
	BOOST_ASSERT (now.time != 0);

	if (m_tLastRecvTime.time == 0)
		return false;

	int timediff = (m_tLastRecvTime.time - now.time) * 1000000 + (m_tLastRecvTime.millitm - now.millitm);

	return timediff > -300000;
}
