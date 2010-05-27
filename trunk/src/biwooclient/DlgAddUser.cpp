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

#ifdef WIN32
#define _WIN32_WINNT 0x0600
#include "winsock2.h"
# include <windows.h>
#endif // WIN32

#include "bcd.h"
#include "DlgAddUser.h"
#include "incdef.h"

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CDlgAddUser, wxDialog)
EVT_BUTTON(wxID_OK, CDlgAddUser::OnLogin)
EVT_BUTTON(wxID_CANCEL, CDlgAddUser::OnCancel)
//EVT_TIMER(wxID_ANY, CDlgAddUser::OnTimer)

EVT_CLOSE(CDlgAddUser::OnClose)

EVT_LISTBOX_DCLICK(Ctrl_ChooseFrom, CDlgAddUser::OnChooseFromListboxDClick)
EVT_LISTBOX_DCLICK(Ctrl_ChooseTo, CDlgAddUser::OnChooseToListboxDClick)
    EVT_TEXT(Ctrl_SearchText, CDlgAddUser::OnTextUpdated)

END_EVENT_TABLE()


class CListItemClientData
	: public wxClientData
{
public:
	CListItemClientData(CUserInfoPointer userInfo)
		: m_userInfo(userInfo)
	{
		BOOST_ASSERT (m_userInfo.get() != NULL);
	}

	CUserInfoPointer userInfo(void) const {return m_userInfo;}

private:
	CUserInfoPointer m_userInfo;
};

CDlgAddUser::CDlgAddUser(wxWindow *parent)
: wxDialog(parent, wxID_ANY, gLangText.dlgAddUserTitle())

{
	BOOST_ASSERT (gMyAccount.get() != NULL);

	wxString sTemp(gAppPath);
	sTemp.Append(wxT("/res/mainframe.ico"));
	wxIcon icon(sTemp, wxBITMAP_TYPE_ICO);
    // set the frame icon
    SetIcon(icon);

	wxBoxSizer *sizerTop = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer * sizerChoose = new wxBoxSizer(wxHORIZONTAL);
    wxSizer * sizerChooseLeft = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, gLangText.dlgStaticChooseFrom()), wxVERTICAL);
    wxSizer * sizerChooseRight = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, gLangText.dlgStaticChooseTo()), wxVERTICAL);
	m_searchTextCtrl = new wxTextCtrl(this, Ctrl_SearchText, "", wxDefaultPosition, wxSize(180, 22));
	m_searchTextCtrl->SetToolTip(gLangText.textSearchTip());
    m_listboxChooseFrom = new wxListBox(this, Ctrl_ChooseFrom, wxDefaultPosition, wxSize(180, 280));
    m_listboxChooseTo = new wxListBox(this, Ctrl_ChooseTo, wxDefaultPosition, wxSize(180, 280));
	sizerChooseLeft->Add(m_searchTextCtrl, 0, wxALL|wxALIGN_CENTER, 5);
    sizerChooseLeft->Add(m_listboxChooseFrom, 1, wxGROW | wxALL, 5);
    sizerChooseRight->Add(m_listboxChooseTo, 1, wxGROW | wxALL, 5);
    sizerChoose->Add(sizerChooseLeft, 1, wxGROW|wxALL, 5 );
    sizerChoose->Add(sizerChooseRight, 1, wxGROW|wxALL, 5 );

    sizerTop->Add(sizerChoose, 1, wxEXPAND | wxLEFT|wxRIGHT, 5 );

	wxSizer *buttonSizer = CreateSeparatedButtonSizer(wxOK | wxCANCEL);
	if ( buttonSizer )
	{
		this->GetWindowChild(wxID_OK)->SetLabel(gLangText.btnAddUserText());
		this->GetWindowChild(wxID_OK)->SetToolTip(gLangText.btnAddUserHelp());
		this->GetWindowChild(wxID_OK)->Disable();
		this->GetWindowChild(wxID_CANCEL)->SetLabel(gLangText.btnCancelText());
		this->GetWindowChild(wxID_CANCEL)->SetToolTip(gLangText.btnCancelHelp());
		sizerTop->Add(buttonSizer, wxSizerFlags().Expand().DoubleBorder());
	}

    SetSizer(sizerTop);
	sizerTop->SetSizeHints(this);
	sizerTop->Fit(this);
}

CDlgAddUser::~CDlgAddUser(void)
{
	delete m_searchTextCtrl;
	delete m_listboxChooseFrom;
	delete m_listboxChooseTo;

	m_existusers.clear();
	m_addusers.clear();
}

void CDlgAddUser::loadContacts(const wxString & searchString)
{
	m_listboxChooseFrom->Clear();

	CCompanyInfoPointer companyInfo = gMyAccount->getUserinfo()->m_companys.begin()->second;
	BOOST_ASSERT (companyInfo.get() != NULL);

	CLockMap<tstring, CUserInfoPointer>::iterator iter;
	for (iter=companyInfo->m_userinfos.begin(); iter!=companyInfo->m_userinfos.end(); iter++)
	{
		CUserInfoPointer userInfo = iter->second;
		if (m_existusers.exist(userInfo->getAccount()))
		{
			continue;
		}

		if (strstr(userInfo->getAccount().c_str(), searchString.c_str()) != NULL ||
			strstr(userInfo->getUserName().c_str(), searchString.c_str()) != NULL ||
			strstr(userInfo->getNick().c_str(), searchString.c_str()) != NULL ||
			strstr(userInfo->getPhone().c_str(), searchString.c_str()) != NULL ||
			strstr(userInfo->getEmail().c_str(), searchString.c_str()) != NULL
			)
		{
			wxString sString = wxString::Format("%s(%s)", userInfo->getUserName().c_str(), userInfo->getAccount().c_str());
			m_listboxChooseFrom->Insert(sString, m_listboxChooseFrom->GetCount(), new CListItemClientData(userInfo));
		}
	}
}

//void CDlgAddUser::OnTimer(wxTimerEvent & event)
//{
//	std::string sAccount = m_textAccount->GetValue();
//	std::string sPassword = m_textPassword->GetValue();
//	doLogin(sAccount, sPassword, true);
//}

void CDlgAddUser::OnLogin(wxCommandEvent& WXUNUSED(event))
{

	wxDialog::EndModal(wxID_OK);		
}

void CDlgAddUser::OnCancel(wxCommandEvent& WXUNUSED(event))
{
	wxDialog::EndModal(wxID_CANCEL);
}

//void CDlgAddUser::OnRegister(wxCommandEvent& WXUNUSED(event))
//{
//
//}

void CDlgAddUser::OnClose(wxCloseEvent& event)
{
	wxDialog::EndModal(wxID_CANCEL);
}

void CDlgAddUser::OnChooseFromListboxDClick(wxCommandEvent& event)
{
    int sel = event.GetInt();

	CListItemClientData * pClientData = (CListItemClientData*)m_listboxChooseFrom->GetClientObject(sel);
	assert (pClientData != NULL);

	CUserInfoPointer userInfo = pClientData->userInfo();
	wxString sString = wxString::Format("%s(%s)", userInfo->getUserName().c_str(), userInfo->getAccount().c_str());

	m_listboxChooseTo->Insert(sString, m_listboxChooseTo->GetCount(), new CListItemClientData(userInfo));
	m_listboxChooseFrom->Delete(sel);

	m_addusers.insert(userInfo->getAccount(), userInfo);
	this->GetWindowChild(wxID_OK)->Enable();
}

void CDlgAddUser::OnChooseToListboxDClick(wxCommandEvent& event)
{
    int sel = event.GetInt();

	CListItemClientData * pClientData = (CListItemClientData*)m_listboxChooseTo->GetClientObject(sel);
	assert (pClientData != NULL);

	CUserInfoPointer userInfo = pClientData->userInfo();
	wxString sString = wxString::Format("%s(%s)", userInfo->getUserName().c_str(), userInfo->getAccount().c_str());

	m_listboxChooseFrom->Insert(sString, m_listboxChooseFrom->GetCount(), new CListItemClientData(userInfo));
	m_listboxChooseTo->Delete(sel);

	m_addusers.remove(userInfo->getAccount());
	if (m_addusers.empty())
	{
		this->GetWindowChild(wxID_OK)->Disable();
	}
}

void CDlgAddUser::OnTextUpdated( wxCommandEvent& event )
{
	if ( event.GetEventType() == wxEVT_COMMAND_TEXT_UPDATED )
	{
		loadContacts(event.GetString());
	}
}
