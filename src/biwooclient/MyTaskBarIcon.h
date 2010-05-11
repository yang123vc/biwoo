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

// MyTaskBarIcon.h file here
#ifndef __MyTaskBarIcon_h__
#define __MyTaskBarIcon_h__

#include "wx/wxprec.h"
#include "wx/taskbar.h"


class MyTaskBarIcon: public wxTaskBarIcon
{
private:
	wxFrame * m_frame;

public:
#if defined(__WXCOCOA__)
	MyTaskBarIcon(wxFrame * framewxTaskBarIconType iconType = DEFAULT_TYPE)
		:   wxTaskBarIcon(iconType)
		, m_frame(frame)
#else
	MyTaskBarIcon(wxFrame * frame)
		: m_frame(frame)
#endif
	{}

	void OnLeftButtonDClick(wxTaskBarIconEvent&);
	void OnMenuRestore(wxCommandEvent&);
	void OnMenuExit(wxCommandEvent&);
	void OnMenuUICheckmark(wxUpdateUIEvent &event);
	virtual wxMenu *CreatePopupMenu();

	DECLARE_EVENT_TABLE()
};

#endif // __MyTaskBarIcon_h__
