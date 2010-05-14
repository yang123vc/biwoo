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
#include "RelationshipWindow.h"

BEGIN_EVENT_TABLE(RelationshipWindow, wxPanel)
EVT_SIZE    (           RelationshipWindow::OnSize)

	EVT_TIMER(wxID_ANY, RelationshipWindow::OnTimer)
    EVT_TEXT(Ctrl_SearchText, RelationshipWindow::OnTextUpdated)

#if USES_TOOLBOOK
#if wxUSE_TOOLBOOK
    EVT_TOOLBOOK_PAGE_CHANGED(wxID_ANY, RelationshipWindow::OnBookCtrl)
    EVT_TOOLBOOK_PAGE_CHANGING(wxID_ANY, RelationshipWindow::OnBookCtrl)
#endif // wxUSE_TOOLBOOK
#else
#if wxUSE_NOTEBOOK
    EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, RelationshipWindow::OnBookCtrl)
    EVT_NOTEBOOK_PAGE_CHANGING(wxID_ANY, RelationshipWindow::OnBookCtrl)
#endif // wxUSE_NOTEBOOK
#endif // USES_TOOLBOOK
END_EVENT_TABLE()

RelationshipWindow::RelationshipWindow(wxWindow * parent)
: wxPanel(parent)
, m_imageList(0)

{
	// ImageList
    const wxSize imageSize(32, 32);

    m_imageList = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
    m_imageList->Add(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, imageSize));
    m_imageList->Add(wxArtProvider::GetIcon(wxART_QUESTION, wxART_OTHER, imageSize));
    m_imageList->Add(wxArtProvider::GetIcon(wxART_WARNING, wxART_OTHER, imageSize));
    m_imageList->Add(wxArtProvider::GetIcon(wxART_ERROR, wxART_OTHER, imageSize));

	m_searchTextCtrl = new wxTextCtrl(this, Ctrl_SearchText);
	m_searchTextCtrl->SetToolTip(gLangText.textSearchTip());

#if USES_TOOLBOOK
    m_bookCtrl = new wxToolbook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_BOTTOM);
#else
    m_bookCtrl = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_BOTTOM);
#endif // USES_TOOLBOOK

    m_bookCtrl->Hide();
	m_bookCtrl->AssignImageList(m_imageList);

    m_rsp0 = new RelationshipPanel(m_bookCtrl);
    m_rsp1 = new RelationshipPanel(m_bookCtrl);
    m_bookCtrl->AddPage( m_rsp0, gLangText.btnMyCompanyText(), false, 0 );
    m_bookCtrl->AddPage( m_rsp1, gLangText.btnMyDepartmentText(), true, 1 );

	gMyCompany = m_rsp0;
	gMyCoGroup = m_rsp1;
	gMsgHandler = m_rsp1;
	m_rsp0->buildAllContacts();
	m_rsp1->buildConversations();

    m_sizerFrame = new wxBoxSizer(wxVERTICAL);
    m_sizerFrame->Insert(0, m_bookCtrl, wxSizerFlags(4).Expand().Border(wxALL, 0));
    m_sizerFrame->Show(m_bookCtrl);
    m_sizerFrame->Layout();

    this->SetSizer(m_sizerFrame);

    m_sizerFrame->SetSizeHints(this);
    m_sizerFrame->Fit(this);
	//m_timer.SetOwner(this);
}

RelationshipWindow::~RelationshipWindow(void)
{
	gMyCompany = NULL;
	gMyCoGroup = NULL;
	gMsgHandler = NULL;

	delete m_searchTextCtrl;
	delete m_bookCtrl;
}

const int constComboHeight = 22;
void RelationshipWindow::OnSize(wxSizeEvent& event)
{
	m_searchTextCtrl->SetSize(event.GetSize().GetWidth(), constComboHeight);
	wxRect rectBookCtrl(0, constComboHeight, event.GetSize().GetWidth(), event.GetSize().GetHeight()-constComboHeight);
	m_bookCtrl->SetSize(rectBookCtrl);
}

void RelationshipWindow::OnTimer(wxTimerEvent & event)
{

}

void RelationshipWindow::OnTextUpdated( wxCommandEvent& event )
{
	if ( event.GetEventType() == wxEVT_COMMAND_TEXT_UPDATED )
	{
		DoSearchUser(event.GetString());
	}
}

#if USES_TOOLBOOK
void RelationshipWindow::OnBookCtrl(wxToolbookEvent& event)
#else
void RelationshipWindow::OnBookCtrl(wxNotebookEvent& event)
#endif // USES_TOOLBOOK
{
    const wxEventType eventType = event.GetEventType();
#if USES_TOOLBOOK
	if ( eventType == wxEVT_COMMAND_TOOLBOOK_PAGE_CHANGED )
#else
	if ( eventType == wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED )
#endif // USES_TOOLBOOK

	{
		RelationshipPanel * currentPage = wx_static_cast(RelationshipPanel*, m_bookCtrl->GetCurrentPage());
		gMsgHandler = currentPage;
	}
#if USES_TOOLBOOK
	else if ( eventType == wxEVT_COMMAND_TOOLBOOK_PAGE_CHANGING )
#else
	else if ( eventType == wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING )
#endif // USES_TOOLBOOK
	{
		//const int idx = event.GetOldSelection();
		//if (m_bookCtrl)
		//{
		//	RelationshipPanel * currentPage = wx_static_cast(RelationshipPanel*, m_bookCtrl->GetPage(event.GetSelection()));
		//	if (currentPage)
		//		currentPage->initSample();
		//}

	}
}

void RelationshipWindow::DoSearchUser(const wxString & searchString)
{
	m_bookCtrl->SetSelection(0);
	m_rsp0->DoSearchUsers(searchString);
}
