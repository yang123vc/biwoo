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
#include "incdef.h"
#include "MyTaskBarIcon.h"
#include "DlgSetting.h"


BEGIN_EVENT_TABLE(MyTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(ID_Menu_Restore, MyTaskBarIcon::OnMenuRestore)
    EVT_MENU(ID_Menu_ChangeAccount,  MyTaskBarIcon::OnChangeAccount)
    EVT_MENU(ID_Menu_InforSetting,  MyTaskBarIcon::OnInformationSetting)
    EVT_MENU(wxID_ABOUT, MyTaskBarIcon::OnAbout)
    EVT_MENU(wxID_EXIT,    MyTaskBarIcon::OnMenuExit)
    EVT_TASKBAR_LEFT_DCLICK(MyTaskBarIcon::OnLeftButtonDClick)
    EVT_UPDATE_UI(ID_Menu_Restore, MyTaskBarIcon::OnMenuUICheckmark)
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

void MyTaskBarIcon::OnChangeAccount(wxCommandEvent& WXUNUSED(event))
{
	m_frame->ProcessCommand(ID_Menu_ChangeAccount);
}

void MyTaskBarIcon::OnInformationSetting(wxCommandEvent& WXUNUSED(event))
{
	m_frame->ProcessCommand(ID_Menu_InforSetting);
}

void MyTaskBarIcon::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	m_frame->ProcessCommand(wxID_ABOUT);
}

void MyTaskBarIcon::OnMenuExit(wxCommandEvent& )
{
	m_frame->ProcessCommand(wxID_EXIT);
}

// Overridables
wxMenu *MyTaskBarIcon::CreatePopupMenu()
{
    // Try creating menus different ways
    // TODO: Probably try calling SetBitmap with some XPMs here
    wxMenu *menu = new wxMenu;
    menu->Append(ID_Menu_Restore, gLangText.menuRestoreText(), gLangText.menuRestoreHelp());
    menu->AppendSeparator();
    menu->Append(ID_Menu_ChangeAccount, gLangText.menuChangeAccText(), gLangText.menuChangeAccHelp());
    menu->Append(ID_Menu_InforSetting, gLangText.menuInforSettingText(), gLangText.menuInforSettingHelp());
    menu->AppendSeparator();
 	menu->Append(wxID_ABOUT, gLangText.menuAboutText(), gLangText.menuAboutHelp());
    menu->AppendSeparator();
    menu->Append(wxID_EXIT, gLangText.menuExitText(), gLangText.menuExitHelp());
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
