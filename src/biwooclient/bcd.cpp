/*
    Biwoo is an enterprise communication software base on CGCP.
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

#include "bcd.h"

tstring gAppPath = _T("");

CAccountInfo::pointer	gMyAccount;
CBiwooClient			m_biwoo;
XmlParseSetting			gSetting;
XmlParseLangText		gLangText;

CbiwooHandler * gAppHandler = NULL;
CbiwooHandler * gMyCompany = NULL;
CbiwooHandler * gMyCoGroup = NULL;
CbiwooHandler * gMsgHandler = NULL;
//CbiwooHandler * gMyFriend = NULL;
//CbiwooHandler * gMyFamily = NULL;
#if 0
#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "libCoderd.lib")
#pragma comment(linker,"/NODEFAULTLIB:libcmtd.lib") 
#else
#pragma comment(lib, "libCoder.lib")
#pragma comment(linker,"/NODEFAULTLIB:libcmt.lib") 
#endif // _DEBUG
#endif // WIN32
#endif