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

// MemberData.h file here
#ifndef __MemberData_h__
#define __MemberData_h__

#include <boost/shared_ptr.hpp>
#include <libRTP/libRTP.h>

class CMemberData
{
public:
	typedef boost::shared_ptr<CMemberData> pointer;
	static CMemberData::pointer create(CRTPData::pointer receiveData, DoRtpHandler::pointer pDoRtpHandler, const void * rtpParam);
	static CMemberData::pointer create(CRTPData::pointer receiveData, DoRtpHandler::pointer pDoRtpHandler, const void * rtpParam, bool silence);

	CRTPData::pointer getRtpData(void) const {return m_receiveData;}
	DoRtpHandler::pointer getDoRtpHandler(void) const {return m_pDoRtpHandler;}
	const void * getRtpParam(void) const {return m_rtpParam;}
	bool isSilence(void) const {return m_silence;}

	//unsigned int addCounter(void) {return ++m_counter;}

public:
	CMemberData(CRTPData::pointer receiveData, DoRtpHandler::pointer pDoRtpHandler, const void * rtpParam, bool silence);

private:
	CRTPData::pointer		m_receiveData;
	DoRtpHandler::pointer	m_pDoRtpHandler;
	const void *			m_rtpParam;
	//unsigned int			m_counter;
	bool					m_silence;
};


#endif // __MemberData_h__
