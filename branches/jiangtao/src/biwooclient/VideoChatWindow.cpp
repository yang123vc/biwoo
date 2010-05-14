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
#include "VideoChatWindow.h"

#include <wx/mimetype.h>
#include <wx/process.h>
#include "incdef.h"

//VideoChatWindow *	gVideoChatWindow = NULL;

BEGIN_EVENT_TABLE(VideoChatWindow, wxPanel)

EVT_SIZE    (           VideoChatWindow::OnSize)
	EVT_TIMER(wxID_ANY, VideoChatWindow::OnTimer)
	EVT_BUTTON(Btn_ReleaseCall, VideoChatWindow::OnReleaseCall)
	EVT_BUTTON(Btn_AcceptVideoCall, VideoChatWindow::OnAcceptVideoCall)

END_EVENT_TABLE()

VideoChatWindow::VideoChatWindow(wxWindow * parent)
: wxPanel(parent)
, m_callid(0)

{
	wxString filename(gAppPath);
	filename.append("/res/biwooman.bmp");
	m_bitmapUI.LoadFile(filename, wxBITMAP_TYPE_BMP);

	m_btnReleaseCall = new wxButton(this, Btn_ReleaseCall, gLangText.btnReleaseCallText());
	m_btnReleaseCall->SetToolTip(gLangText.btnReleaseCallHelp());
	m_btnAcceptVideoCall = new wxButton(this, Btn_AcceptVideoCall, gLangText.btnAcceptCallText());
	m_btnAcceptVideoCall->SetToolTip(gLangText.btnAcceptCallHelp());

	m_btnReleaseCall->Show(false);
	m_btnAcceptVideoCall->Show(false);

	m_staLocal = new wxStaticBitmap(this, wxID_ANY, m_bitmapUI);
	m_staRemote = new wxStaticBitmap(this, wxID_ANY, m_bitmapUI);

	m_timer.SetOwner(this);
}

VideoChatWindow::~VideoChatWindow(void)
{
	delete m_btnReleaseCall;
	delete m_btnAcceptVideoCall;
	delete m_staLocal;
	delete m_staRemote;
}

HWND VideoChatWindow::GetLocalHwnd(void) const
{
	return m_staLocal == NULL ? NULL : (HWND)m_staLocal->GetHandle();
}

HWND VideoChatWindow::GetRemoteHwnd(void) const
{
	return m_staRemote == NULL ? NULL : (HWND)m_staRemote->GetHandle();
}

void VideoChatWindow::videoCall(CUserInfo::pointer userInfo)
{
	assert (userInfo.get() != NULL);
	m_userInfo = userInfo;

	m_btnReleaseCall->Show(true);
	m_btnAcceptVideoCall->Show(false);

	HWND hWndPreview = GetRemoteHwnd();
	m_biwoo.videoCall(m_userInfo, hWndPreview);

}

void VideoChatWindow::SetVideoCall(bool ownerMsg, CUserInfo::pointer userInfo, long callid)
{
	assert (userInfo.get() != NULL);
	m_userInfo = userInfo;
	m_callid = callid;

	m_btnReleaseCall->Show(true);
	if (!ownerMsg)
	{
		m_btnAcceptVideoCall->Show(true);
	}
}

const short lHeight = 80;
const short lWidth = lHeight * 4 / 3;

void VideoChatWindow::OnSize(wxSizeEvent& event)
{
	wxSize size = event.GetSize();
	int rvideowidth = size.GetHeight() * 4 / 3;

	wxPoint rpt((size.GetWidth()-rvideowidth)/2, 0);
	wxSize rvideosize(rvideowidth, size.GetHeight());

	if (m_btnReleaseCall != NULL)
	{
		m_btnReleaseCall->Move(0, 0);
	}
	if (m_btnAcceptVideoCall != NULL)
	{
		m_btnAcceptVideoCall->Move(0, 30);
	}

	if (m_staRemote != NULL)
	{
		m_staRemote->SetSize(wxRect(rpt, rvideosize));
		m_staRemote->Refresh();
	}

	if (m_staLocal != NULL)
	{
		wxPoint lpt;
		wxSize lvideosize(lWidth, lHeight);
		if (rpt.x > lWidth)
		{
			lpt = wxPoint(rpt.x-lWidth, size.GetHeight()-lHeight);
		}else
		{
			lpt = wxPoint(0, size.GetHeight()-lHeight);
		}

		m_staLocal->SetSize(wxRect(lpt, lvideosize));
		m_staLocal->Refresh();
	}

	m_timer.Start(100, true);
}

void VideoChatWindow::OnTimer(wxTimerEvent & event)
{
	m_biwoo.moveRemoteWindow();

	//if (gLDoDSHandler != NULL)
	//{
	//	gLDoDSHandler->MoveWindow();
	//}
}

void VideoChatWindow::OnReleaseCall(wxCommandEvent& event)
{
	m_biwoo.releaseVideoCall(m_userInfo, m_callid);
	m_biwoo.closeLocalAV();
	m_callid = 0;
}

void VideoChatWindow::OnAcceptVideoCall(wxCommandEvent& event)
{
	HWND hLWndPreview = GetLocalHwnd();
	HWND hRWndPreview = GetRemoteHwnd();

	m_biwoo.openLocalAV(hLWndPreview);
	m_biwoo.acceptVideoCall(m_userInfo, hRWndPreview, m_callid);

	m_btnAcceptVideoCall->Show(false);
}
