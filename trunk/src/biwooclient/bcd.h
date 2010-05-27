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

// bcd.h file here
#ifndef __bcd_h__
#define __bcd_h__

#include <stl/locklist.h>
#include <stl/lockmap.h>
#include "../biwoolib/biwoolib.h"
#include "XmlParseSetting.h"
#include "XmlParseLangText.h"

extern tstring gAppPath;

extern CAccountInfo::pointer	gMyAccount;
extern CBiwooClient		m_biwoo;

extern XmlParseSetting	gSetting;
extern XmlParseLangText	gLangText;

//////////////////////////////////////
extern CbiwooHandler * gAppHandler;
extern CbiwooHandler * gMyCompany;
extern CbiwooHandler * gMyCoGroup;
extern CbiwooHandler * gMsgHandler;
//extern CbiwooHandler * gMyFriend;
//extern CbiwooHandler * gMyFamily;

const tstring BIWOO_NAME = "biwoo";

#endif // __bcd_h__
