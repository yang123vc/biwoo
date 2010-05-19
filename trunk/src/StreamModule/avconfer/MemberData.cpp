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

#include "MemberData.h"

CMemberData::pointer CMemberData::create(CRTPData::pointer receiveData, DoRtpHandler::pointer pDoRtpHandler, const void * rtpParam)
{
	return CMemberData::pointer(new CMemberData(receiveData, pDoRtpHandler, rtpParam, false));
}

CMemberData::pointer CMemberData::create(CRTPData::pointer receiveData, DoRtpHandler::pointer pDoRtpHandler, const void * rtpParam, bool silence)
{
	return CMemberData::pointer(new CMemberData(receiveData, pDoRtpHandler, rtpParam, silence));
}

CMemberData::CMemberData(CRTPData::pointer receiveData, DoRtpHandler::pointer pDoRtpHandler, const void * rtpParam, bool silence)
: m_receiveData(receiveData)
, m_pDoRtpHandler(pDoRtpHandler)
, m_rtpParam(rtpParam)
//, m_counter(0)
, m_silence(silence)

{
}