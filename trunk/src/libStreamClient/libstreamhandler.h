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

// libstreamhandler.h file here
#ifndef __libstreamhandler_h__
#define __libstreamhandler_h__

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "stl/stldef.h"
#include "CGCLib/CGCLib.h"

class CP2PHandler;

class CDoP2PClientHandler
	: public DoSotpClientHandler
	, public boost::enable_shared_from_this<CDoP2PClientHandler>
{
public:
	enum P2P_Type
	{
		P2P_MSG
		, P2P_AUDIO	= 0x11
		, P2P_VIDEO	= 0x12
		, P2P_FILE	= 0x21
	};

	typedef boost::shared_ptr<CDoP2PClientHandler> pointer;

	virtual void setP2PHandler(CP2PHandler * handler, bool disableSotpParse) = 0;

	virtual const tstring & getP2PUser(void) const = 0;
	virtual long getP2PType(void) const = 0;
	virtual void setP2PParam(long newV) = 0;
	virtual long getP2PParam(void) const = 0;

	virtual void setP2PAddress(const tstring & newV) = 0;
	virtual const tstring & getP2PAddress(void) const = 0;

	virtual void setP2PInfo(const tstring & sP2PUser, long nP2PType) = 0;

	virtual void clearP2PStatus(void) = 0;
	virtual void setLocalP2PStatus(bool newv) = 0;
	virtual bool getLocalP2PStatus(void) const = 0;
	virtual void setRemoteP2PStatus(bool newv) = 0;
	virtual bool getRemoteP2PStatus(void) const = 0;
	virtual DoSotpClientHandler::pointer dohandler(void) const = 0;

};

class CP2PHandler
{
public:
	virtual void onP2PEvent(CDoP2PClientHandler::pointer p2pClient, const cgcParser & response) = 0;
	virtual void onP2PEvent(CDoP2PClientHandler::pointer p2pClient, CCgcData::pointer receiveData) = 0;
};

class CStreamHandler
{
public:
	virtual void onUserInfo(const tstring & sUserId, const tstring & sUserName) {}
	virtual void onUserLogined(const tstring & sUserName) {}
	virtual void onUserLogouted(const tstring & sUserName) {}

	virtual void onP2PRequestResult(const tstring & sFromUser, long nP2PType, long resuleValue) {}
	virtual CP2PHandler * onP2PUserAck(CDoP2PClientHandler::pointer p2pClient, bool & disableSotpParse) {return 0;}	// disableSotpParse : default true
	virtual void onP2PRequestTimeout(CDoP2PClientHandler::pointer p2pClient) {}
	virtual void onP2PUserDisconnect(CDoP2PClientHandler::pointer p2pClient) {}

};


#endif // __libstreamhandler_h__
