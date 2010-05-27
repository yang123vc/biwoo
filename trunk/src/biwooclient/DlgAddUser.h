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

// DlgAddUser.h file here
#ifndef __DlgAddUser_h__
#define __DlgAddUser_h__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
 
#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// A custom modeless dialog
class CDlgAddUser : public wxDialog
{
public:
    CDlgAddUser(wxWindow *parent);
	~CDlgAddUser(void);

	CLockMap<tstring, bool> m_existusers;
	CLockMap<tstring, CUserInfo::pointer> m_addusers;

	void loadContacts(const wxString & sSearch = "");

private:

    void OnLogin(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
	//void OnTimer(wxTimerEvent & event);
	void OnChooseFromListboxDClick(wxCommandEvent& event);
	void OnChooseToListboxDClick(wxCommandEvent& event);
	void OnTextUpdated( wxCommandEvent& event );

    DECLARE_EVENT_TABLE()

private:
	wxTextCtrl * m_searchTextCtrl;
	wxListBox * m_listboxChooseFrom;
	wxListBox * m_listboxChooseTo;
	//wxButton * m_btnQuitDialog;
};


#endif // __DlgAddUser_h__
