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

// libstreamxhandler.h file here
#ifndef __libstreamxhandler_h__
#define __libstreamxhandler_h__

#include "stl/stldef.h"

class OnP2PHandler
{
public:
	virtual void onUserInfo(const tstring & sUserId, const tstring & sUserName) {}
	virtual void onUserLogined(const tstring & sUserName) {}
	virtual void onUserLogouted(const tstring & sUserName) {}

	virtual void onP2PRequestResult(const tstring & sFromUser, long nP2PType, long resuleValue) {}
	virtual void onP2PRequestTimeout(const tstring & sFromUser, long nP2PType) {}
	virtual void onP2PUserConnected(const tstring & sFromUser, long nP2PType, long nP2PParam) {}
	virtual void onP2PUserDisconnected(const tstring & sFromUser, long nP2PType, long nP2PParam) {}

	// File handler
	virtual void onFileRequest(const tstring & sFromUser, long fid, const tstring & filename, size_t filesize) {}
	virtual void onFileRejected(const tstring & sFromUser, long fid) {}
	virtual void onFileCanceled(const tstring & sFromUser, long fid) {}
	virtual void onSendFileData(const tstring & sFromUser, long fid, float percent) {}
	virtual void onReceiveFileData(const tstring & sFromUser, long fid, float percent) {}
};

#endif // __libstreamxhandler_h__
