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
#include "GroupChatWindow.h"

#include <wx/mimetype.h>
#include <wx/process.h>
#include "incdef.h"

//GroupChatWindow *	gGroupChatWindow = NULL;

BEGIN_EVENT_TABLE(GroupChatWindow, wxPanel)

EVT_SIZE    (           GroupChatWindow::OnSize)

END_EVENT_TABLE()

GroupChatWindow::GroupChatWindow(wxWindow * parent)
: wxPanel(parent)

{
	wxString filename(gAppPath);
	filename.append("/res/biwooman.bmp");
	m_bitmapUI.LoadFile(filename, wxBITMAP_TYPE_BMP);

    m_imageListNormal = new wxImageList(32, 32, true);
    //m_imageListSmall = new wxImageList(16, 16, true);

	m_imageListNormal->Add(wxIcon(wxString::Format("%s/res/icon_man.ico", gAppPath.c_str()), wxBITMAP_TYPE_ICO));

	m_listCtrl = new wxListCtrl(this, Ctrl_GroupContacts, wxDefaultPosition, wxDefaultSize, wxLC_ICON|wxSUNKEN_BORDER);
	m_listCtrl->AssignImageList(m_imageListNormal, wxIMAGE_LIST_NORMAL);
    //m_listCtrl->AssignImageList(m_imageListSmall, wxIMAGE_LIST_SMALL);

}

GroupChatWindow::~GroupChatWindow(void)
{
	delete m_listCtrl;
}

void GroupChatWindow::OnSize(wxSizeEvent& event)
{
	m_listCtrl->SetSize(event.GetSize());
}

void GroupChatWindow::cleanUser(void)
{
	m_listCtrl->ClearAll();
}

void GroupChatWindow::invitedUser(CUserInfo::pointer userInfo)
{
	assert (userInfo.get() != NULL);

	long item = m_listCtrl->InsertItem(m_listCtrl->GetItemCount(), wxString::Format(_T("%s(%s)"), userInfo->getUserName().c_str(), userInfo->getAccount().c_str()), 0);
	//m_listCtrl->SetItemData(item, );
}


