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

// GroupChatWindow.h file here
#ifndef __GroupChatWindow_h__
#define __GroupChatWindow_h__

#include "wx/wxprec.h"
#include "wx/listctrl.h"

class GroupChatWindow
	: public wxPanel
{
public:
	GroupChatWindow(wxWindow * parent);
	~GroupChatWindow(void);

public:
	void cleanUser(void);
	void invitedUser(CUserInfo::pointer userInfo);
	//void SetVideoCall(bool ownerMsg, CUserInfo::pointer userInfo, long callid);

protected:

private:
    wxImageList *m_imageListNormal;
    wxImageList *m_imageListSmall;
	wxListCtrl * m_listCtrl;
	wxBitmap m_bitmapUI;

	void OnSize(wxSizeEvent& event);
	void OnTimer(wxTimerEvent & event);

	DECLARE_EVENT_TABLE()

};

//extern GroupChatWindow *	gGroupChatWindow;

#endif // __GroupChatWindow_h__
