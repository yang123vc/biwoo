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

// ConferenceMember.h file here
#ifndef __ConferenceMember_h__
#define __ConferenceMember_h__

#include <boost/thread.hpp>
#include "../../ThirdParty/stl/stldef.h"
#include "../../ThirdParty/stl/buffer.h"
#include <boost/shared_ptr.hpp>
#include <CGCBase/cgcRtp.h>
#include <sys/timeb.h>

class CConferenceMember
{
public:
	typedef boost::shared_ptr<CConferenceMember> pointer;
	static CConferenceMember::pointer create(DoRtpHandler::pointer audio, DoRtpHandler::pointer video);

	DoRtpHandler::pointer getAudioHandler(void) const {return m_audioHandler;}
	DoRtpHandler::pointer getVideoHandler(void) const {return m_videoHandler;}
	void clearAVHandler(void) {m_audioHandler.reset(); m_videoHandler.reset();}

	void setIp(const tstring & newv) {m_ip = newv;}
	const tstring & getIp(void) const {return m_ip;}

	void setAudioPort(int newv) {m_audioPort = newv;}
	int getAudioPort(void) const {return m_audioPort;}

	void setVideoPort(int newv) {m_videoPort = newv;}
	int getVideoPort(void) const {return m_videoPort;}

	void setAudioSend(bool newv) {m_audioSend = newv;}
	bool getAudioSend(void) const {return m_audioSend;}
	void setAudioRecv(bool newv) {m_audioRecv = newv;}
	bool getAudioRecv(void) const {return m_audioRecv;}
	void setVideoSend(bool newv) {m_videoSend = newv;}
	bool getVideoSend(void) const {return m_videoSend;}
	void setVideoRecv(bool newv) {m_videoRecv = newv;}
	bool getVideoRecv(void) const {return m_videoRecv;}
	int getRecvIndex(void) {return m_recvIndex;} //add by xap
	void setRecvIndex(int index) {m_recvIndex = index;} //add by xap 

	void setClosed(bool newv) {m_bClosed = newv;}
	bool getClosed(void) const {return m_bClosed;}
	void setIndex(int newv) {m_index = newv;}
	int getIndex(void) const {return m_index;}

	void setRecvDataTime(void);
	bool hasRecvData(struct timeb & now);

	CBuffer::pointer audioBuffer(void) const {return m_audioBuffer;}

public:
	CConferenceMember(DoRtpHandler::pointer audio, DoRtpHandler::pointer video);

private:
	DoRtpHandler::pointer m_audioHandler;
	DoRtpHandler::pointer m_videoHandler;
	tstring m_ip;
	int m_audioPort;
	int m_videoPort;

	bool m_audioSend;	// default true
	bool m_audioRecv;	// default true
	bool m_videoSend;	// default false
	bool m_videoRecv;	// default false
	int m_recvIndex;    // add by xap 接收列表

	bool m_bClosed;		// default false
	int m_index;
	struct timeb m_tLastRecvTime;
	CBuffer::pointer m_audioBuffer;
};


#endif // __ConferenceMember_h__
