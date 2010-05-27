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
#include "MyTaskBarIcon.h"

enum {
    PU_RESTORE = 10001,
    //PU_EXIT,
};


BEGIN_EVENT_TABLE(MyTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(PU_RESTORE, MyTaskBarIcon::OnMenuRestore)
    EVT_MENU(wxID_EXIT,    MyTaskBarIcon::OnMenuExit)
    EVT_TASKBAR_LEFT_DCLICK  (MyTaskBarIcon::OnLeftButtonDClick)
    EVT_UPDATE_UI(PU_RESTORE,MyTaskBarIcon::OnMenuUICheckmark)
END_EVENT_TABLE()

void MyTaskBarIcon::OnMenuRestore(wxCommandEvent& )
{
	if (m_frame->IsVisible())
	{
		m_frame->Show(false);
	}else
	{
		m_frame->Show(true);
		m_frame->Restore();
	}
}

void MyTaskBarIcon::OnMenuExit(wxCommandEvent& )
{
	if (wxMessageBox(gLangText.menuExitQuestion(), BIWOO_NAME, wxICON_QUESTION | wxYES_NO, m_frame) == wxYES)
	{
		m_frame->Destroy();
	}
}

// Overridables
wxMenu *MyTaskBarIcon::CreatePopupMenu()
{
    // Try creating menus different ways
    // TODO: Probably try calling SetBitmap with some XPMs here
    wxMenu *menu = new wxMenu;
    menu->Append(PU_RESTORE, gLangText.menuRestoreText(), gLangText.menuRestoreHelp());
    menu->AppendSeparator();
    menu->Append(wxID_EXIT, gLangText.menuExitText(), gLangText.menuExitHelp());
    //menu->Append(PU_EXIT,    _T("E&xit"));
    return menu;
}

void MyTaskBarIcon::OnLeftButtonDClick(wxTaskBarIconEvent&)
{
	m_frame->Show(true);
	m_frame->Restore();
}

void MyTaskBarIcon::OnMenuUICheckmark(wxUpdateUIEvent &event)
{
	if (m_frame->IsVisible())
	{
		event.SetText(gLangText.menuHideText());
	}else
	{
		event.SetText(gLangText.menuRestoreText());
	}
}
