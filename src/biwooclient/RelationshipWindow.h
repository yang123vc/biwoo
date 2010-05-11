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

// RelationshipWindow.h file here
#ifndef __RelationshipWindow_h__
#define __RelationshipWindow_h__

#include "wx/wxprec.h"
#include "wx/toolbook.h"
#include "wx/artprov.h"
#include "RelationshipPanel.h"
#include "incdef.h"

#define USES_TOOLBOOK	1 // [0,1]

class RelationshipWindow
	: public wxPanel
{
private:
	wxTextCtrl * m_searchTextCtrl;
	wxBookCtrlBase * m_bookCtrl;
	wxImageList * m_imageList;
    wxBoxSizer * m_sizerFrame;
	RelationshipPanel * m_rsp0;		// My Company
	RelationshipPanel * m_rsp1;		// My Department
	//wxTimer m_timer;

public:
	RelationshipWindow(wxWindow * parent);
	~RelationshipWindow(void);

private:
	void DoSearchUser(const wxString & searchString);

	void OnSize(wxSizeEvent& event);
	void OnTimer(wxTimerEvent & event);
    void OnTextUpdated( wxCommandEvent& event );

#if USES_TOOLBOOK
    void OnBookCtrl(wxToolbookEvent& event);
#else
    void OnBookCtrl(wxNotebookEvent& event);
#endif // USES_TOOLBOOK

    DECLARE_EVENT_TABLE()

};

#endif // __RelationshipWindow_h__
