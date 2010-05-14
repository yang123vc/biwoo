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

// AVSProxy.h file here
#ifndef __AVSProxy_h__
#define __AVSProxy_h__

//#include <stl/lockmap.h>
#include "UserInfo.h"
//#include "CommConferMgr.h"
#include "avconfer/AVConference.h"

const int NAME_USERINFO = 100;

class CAVSProxy
{
private:

public:
	void clearAll(void);

public:
	//CLockMap<tstring, CUserInfo::pointer> m_mapUserInfo;
	//CCommConferMgr	m_commfMgr;
	CAVConference	m_conference;

public:
	CAVSProxy(void);
	~CAVSProxy(void);

};


extern CAVSProxy gAVSProxy;

#endif // __AVSProxy_h__
