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

// incdef.h file here
#ifndef __incdef_h__
#define __incdef_h__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
 
#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#if defined(__WXMOTIF__) || defined(__WXPM__) || defined(__WXX11__) || defined(__WXMGL__)
    #define USE_CONTEXT_MENU 0
#else
    #define USE_CONTEXT_MENU 1
#endif


// control ids
enum
{
	Menu_NewRootGroup		= 100
	, Menu_Deletemessage
	, ID_Menu_ChangeAccount
	, Menu_NewChildGroup
	, Menu_DeleteGroup

	, Btn_SendMsg			= 1000
	, Btn_SendFile
	, Btn_VideoCall
	, Btn_ReleaseCall
	, Btn_AcceptVideoCall
	, Btn_AddUser
	, Btn_QuitDialog

	, TreeTest_Ctrl			= 1100

	, Ctrl_ChooseFrom		= 1200
	, Ctrl_ChooseTo
	, Ctrl_GroupContacts
	, Ctrl_SearchText

};

#endif // __incdef_h__
