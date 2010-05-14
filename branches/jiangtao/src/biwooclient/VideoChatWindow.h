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

// VideoChatWindow.h file here
#ifndef __VideoChatWindow_h__
#define __VideoChatWindow_h__

#include "wx/wxprec.h"

class VideoChatWindow
	: public wxPanel
{
public:
	VideoChatWindow(wxWindow * parent);
	~VideoChatWindow(void);

public:
	HWND GetLocalHwnd(void) const;
	HWND GetRemoteHwnd(void) const;

	void videoCall(CUserInfo::pointer userInfo);
	void SetVideoCall(bool ownerMsg, CUserInfo::pointer userInfo, long callid);

protected:

private:
	wxBitmap m_bitmapUI;
	wxButton * m_btnReleaseCall;
	wxButton * m_btnAcceptVideoCall;
	wxStaticBitmap * m_staLocal;
	wxStaticBitmap * m_staRemote;
	wxTimer m_timer;

	CUserInfo::pointer	m_userInfo;
	long				m_callid;

	void OnReleaseCall(wxCommandEvent& event);
	void OnAcceptVideoCall(wxCommandEvent& event);

	void OnSize(wxSizeEvent& event);
	void OnTimer(wxTimerEvent & event);

	DECLARE_EVENT_TABLE()

};

//extern VideoChatWindow *	gVideoChatWindow;

#endif // __VideoChatWindow_h__
