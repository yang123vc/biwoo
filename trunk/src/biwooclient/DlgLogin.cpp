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
#include <bodb/bodb2.h>
#include <bodb/fieldvariant.h>
using namespace bo;

#include "bcd.h"
#include "DlgLogin.h"

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CDlglogin, wxDialog)
EVT_BUTTON(wxID_OK, CDlglogin::OnLogin)
EVT_BUTTON(wxID_CANCEL, CDlglogin::OnCancel)
//EVT_BUTTON(ID_REGISTER, CDlglogin::OnRegister)
EVT_TIMER(wxID_ANY, CDlglogin::OnTimer)

EVT_CLOSE(CDlglogin::OnClose)
END_EVENT_TABLE()

wxStaticText * staAccount = NULL;
wxStaticText * staPassword = NULL;

CDlglogin::CDlglogin(wxWindow *parent, bool changeAccount)
: wxDialog(parent, wxID_ANY, gLangText.dlgLoginTitle())
, m_textAccount(0), m_textPassword(0)
, m_cgcpReturned(false), m_errorCode(0)
, m_changeAccount(changeAccount)

{
	wxString sTemp(gAppPath);
	sTemp.Append(wxT("/res/mainframe.ico"));
	wxIcon icon(sTemp, wxBITMAP_TYPE_ICO);
    // set the frame icon
    SetIcon(icon);

	m_biwoo.setHandler((CbiwooHandler*)this);

	wxBoxSizer *sizerTop = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer * sizerAccount = new wxBoxSizer(wxHORIZONTAL);
	staAccount = new wxStaticText(this, wxID_ANY, gLangText.dlgLoginAccountText());
	m_textAccount = new wxTextCtrl(this, wxID_ANY);
	m_textAccount->SetToolTip(gLangText.dlgLoginAccountHelp());
	sizerAccount->Add(staAccount, 1, wxALL|wxALIGN_CENTER, 5);
	sizerAccount->Add(m_textAccount, 1, wxALL|wxALIGN_CENTER, 5);

	wxBoxSizer * sizerPassword = new wxBoxSizer(wxHORIZONTAL);
	staPassword = new wxStaticText(this, wxID_ANY, gLangText.dlgLoginPasswordText());
	m_textPassword = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	m_textPassword->SetToolTip(gLangText.dlgLoginPasswordHelp());
	sizerPassword->Add(staPassword, 1, wxALL|wxALIGN_CENTER, 5);
	sizerPassword->Add(m_textPassword, 1, wxALL|wxALIGN_CENTER, 5);

	wxBoxSizer * sizerCheckBox = new wxBoxSizer(wxHORIZONTAL);
	m_checkSavePassword = new wxCheckBox(this, wxID_ANY, gLangText.checkSavePassword());
	m_checkAutoLogin = new wxCheckBox(this, wxID_ANY, gLangText.checkAutoLogin());
	//m_checkAutoLogin->Disable();
	sizerCheckBox->Add(m_checkSavePassword, 1, wxALL|wxALIGN_CENTER, 5);
	sizerCheckBox->Add(m_checkAutoLogin, 1, wxALL|wxALIGN_CENTER, 5);

    sizerTop->Add(sizerAccount, 0, wxEXPAND | wxLEFT|wxRIGHT, 5 );
    sizerTop->Add(sizerPassword, 0, wxEXPAND | wxLEFT|wxRIGHT, 5 );
    sizerTop->Add(sizerCheckBox, 0, wxEXPAND | wxLEFT|wxRIGHT, 5 );

	wxSizer *buttonSizer = CreateSeparatedButtonSizer(wxOK | wxCANCEL);
	if ( buttonSizer )
	{
		this->GetWindowChild(wxID_OK)->SetLabel(gLangText.btnLoginText());
		this->GetWindowChild(wxID_OK)->SetToolTip(gLangText.btnLoginHelp());
		this->GetWindowChild(wxID_CANCEL)->SetLabel(gLangText.btnCancelText());
		this->GetWindowChild(wxID_CANCEL)->SetToolTip(gLangText.btnCancelHelp());
		sizerTop->Add(buttonSizer, wxSizerFlags().Expand().DoubleBorder());
	}

    SetSizer(sizerTop);
	sizerTop->SetSizeHints(this);
	sizerTop->Fit(this);

	loadDbInfo();
}

CDlglogin::~CDlglogin(void)
{
	delete staAccount;
	delete m_textAccount;
	delete staPassword;
	delete m_textPassword;
	delete m_checkSavePassword;
	delete m_checkAutoLogin;
	m_records.clear();
}

void CDlglogin::loadDbInfo(void)
{
	wxString sPathUser(gAppPath);
	sPathUser.append("/default");
	if (!bodb_init(sPathUser.c_str()))
	{
		return;
	}
	if (bodb_exec("USE bocinfo") != 0)
	{
		return;
	}

	char sqlBuffer[1024];
	memset(sqlBuffer, 0, sizeof(sqlBuffer));
	sprintf(sqlBuffer, "SELECT account,password FROM loginrecords_t");
	PRESULTSET resultset = 0;
	bodb_exec(sqlBuffer, &resultset);
	if (resultset != 0)
	{
		for (int i=0; i<resultset->rscount; i++)
		{
			CFieldVariant varAccount(resultset->rsvalues[i]->fieldvalues[0]);
			CFieldVariant varPassword(resultset->rsvalues[i]->fieldvalues[1]);

			CUserInfo::pointer userInfo = CUserInfo::create(varAccount.getString(), varPassword.getString());
			m_records.insert(userInfo->getAccount(), userInfo);
		}

		bodb_free(resultset);
	}

	memset(sqlBuffer, 0, sizeof(sqlBuffer));
	sprintf(sqlBuffer, "SELECT lastaccount,autologin FROM bocinfo_t");
	bodb_exec(sqlBuffer, &resultset);
	if (resultset != 0)
	{
		for (int i=0; i<resultset->rscount; i++)
		{
			CFieldVariant varLastAccount(resultset->rsvalues[i]->fieldvalues[0]);
			CFieldVariant varAutoLogin(resultset->rsvalues[i]->fieldvalues[1]);
			std::string sAccount = varLastAccount.getString();

			m_textAccount->SetValue(sAccount);
			m_checkAutoLogin->SetValue(varAutoLogin.getBool());

			CUserInfo::pointer userInfo;
			if (m_records.find(sAccount, userInfo))
			{
				m_textPassword->SetValue(userInfo->getPassword());
				m_checkSavePassword->SetValue(!userInfo->getPassword().empty());
			}

			break;
		}

		bodb_free(resultset);
	}

	if (!m_changeAccount && m_checkAutoLogin->IsChecked())
	{
		m_timer.SetOwner(this);
		m_timer.Start(200, true);
	}
}

void CDlglogin::OnTimer(wxTimerEvent & event)
{
	std::string sAccount = m_textAccount->GetValue();
	std::string sPassword = m_textPassword->GetValue();
	doLogin(sAccount, sPassword, true);
}

bool CDlglogin::doLogin(const std::string & sAccount, const std::string & sPassword, bool autoLogin)
{
	if (sAccount.empty())
	{
		return false;
	}

	CCgcAddress serverAddress(gSetting.address(), CCgcAddress::ST_UDP);
	//CCgcAddress fileserverAddress(gSetting.fileserver(), CCgcAddress::ST_UDP);
	//CCgcAddress rtpserverAddress(gSetting.p2prtp(), CCgcAddress::ST_RTP);
	//CCgcAddress udpserverAddress(gSetting.p2pudp(), CCgcAddress::ST_UDP);
	if (!m_biwoo.start(serverAddress, (CbiwooHandler*)this))
	{
		m_biwoo.stop();
		wxMessageBox(gLangText.textConnectError(), gLangText.dlgLoginTitle(), wxOK | wxICON_ERROR, this);
		return false;
	}

	m_cgcpReturned = false;
	this->GetWindowChild(wxID_OK)->Disable();
	m_biwoo.accountRegister(sAccount.c_str(), sPassword.c_str());

	short i=0;
	while (++i < 50)
	{
		if (m_cgcpReturned)
			break;
#ifdef WIN32
		Sleep(100);
#else
		usleep(100000);
#endif
	}

	if (m_accountInfo.get() != 0)
	{
		if (!autoLogin && bodb_isopened())
		{
			char sql[2048];
			memset(sql, 0, sizeof(sql));
			int ret = bodb_exec("SELECT * FROM bocinfo_t");
			if (ret < 1)
			{
				sprintf(sql, "INSERT INTO bocinfo_t(lastaccount,autologin) VALUES('%s',%s)", sAccount.c_str(), m_checkAutoLogin->IsChecked() ? "true" : "false");
			}else
			{
				sprintf(sql, "UPDATE bocinfo_t SET lastaccount='%s',autologin=%s", sAccount.c_str(), m_checkAutoLogin->IsChecked() ? "true" : "false");
			}
			bodb_exec(sql);

			memset(sql, 0, sizeof(sql));
			sprintf(sql, "SELECT * FROM loginrecords_t WHERE account='%s'", sAccount.c_str());
			ret = bodb_exec(sql);

			std::string sPasswordSave = m_checkSavePassword->IsChecked() ? sPassword : "";
			if (ret < 1)
			{
				sprintf(sql, "INSERT INTO loginrecords_t(account,password) VALUES('%s','%s')", sAccount.c_str(), sPasswordSave.c_str());
			}else
			{
				sprintf(sql, "UPDATE loginrecords_t SET password='%s' WHERE account='%s'", sPasswordSave.c_str(), sAccount.c_str());
			}
			bodb_exec(sql);
		}

		wxDialog::EndModal(wxID_OK);		
	}else
	{
		m_textAccount->SetFocus();
		m_textAccount->SetSelection(0, 0);
		this->GetWindowChild(wxID_OK)->Enable();
		//m_btnRegister->Enable();

		if (!m_cgcpReturned)
		{
			wxMessageBox(gLangText.textLoginTimeout(), gLangText.dlgLoginTitle(), wxOK | wxICON_ERROR, this);
		}else
		{
			switch (m_errorCode)
			{
			case 11:
			case 13:
				wxMessageBox(gLangText.textAccOrPwdError(), gLangText.dlgLoginTitle(), wxOK | wxICON_ERROR, this);
				break;
			default:
				wxString message;
				message.Format(_T("%s (%d)"), gLangText.textSystemError().c_str(), m_errorCode);
				wxMessageBox(message, gLangText.dlgLoginTitle(), wxOK | wxICON_ERROR, this);
				break;
			}
		}
	}

	return m_accountInfo.get() != NULL;
}

void CDlglogin::OnLogin(wxCommandEvent& WXUNUSED(event))
{
	if (m_textAccount->IsEmpty())
	{
		m_textAccount->SetFocus();
		return;
	}

	std::string sAccount = m_textAccount->GetValue();
	std::string sPassword = m_textPassword->GetValue();
	doLogin(sAccount, sPassword, false);

	/*
	CCgcAddress serverAddress(gSetting.address(), CCgcAddress::ST_UDP);
	CCgcAddress fileserverAddress(gSetting.fileserver(), CCgcAddress::ST_UDP);
	CCgcAddress rtpserverAddress(gSetting.p2prtp(), CCgcAddress::ST_RTP);
	CCgcAddress udpserverAddress(gSetting.p2pudp(), CCgcAddress::ST_UDP);
	if (!m_biwoo.start(serverAddress, fileserverAddress, rtpserverAddress, udpserverAddress, (CbiwooHandler*)this))
	{
		m_biwoo.stop();
		wxMessageBox(gLangText.textConnectError(), gLangText.dlgLoginTitle(), wxOK | wxICON_ERROR, this);
		return;
	}
	if (m_textAccount->IsEmpty())
	{
		m_textAccount->SetFocus();
		return;
	}
	wxString sAccount = m_textAccount->GetValue();
	wxString sPassword = m_textPassword->GetValue();

	m_cgcpReturned = false;
	m_btnLogin->Disable();
	//m_btnRegister->Disable();
	m_biwoo.accountRegister(sAccount.c_str(), sPassword.c_str());

	short i=0;
	while (++i < 50)
	{
		if (m_cgcpReturned)
			break;
#ifdef WIN32
		Sleep(100);
#else
		usleep(100000);
#endif
	}

	if (m_accountInfo.get() != 0)
	{
		if (bodb_isopened())
		{
			char sql[2048];
			memset(sql, 0, sizeof(sql));
			int ret = bodb_exec("SELECT * FROM bocinfo_t");
			if (ret < 1)
			{
				sprintf(sql, "INSERT INTO bocinfo_t(lastaccount,autologin) VALUES('%s',%s)", sAccount.c_str(), m_checkAutoLogin->IsChecked() ? "true" : "false");
			}else
			{
				sprintf(sql, "UPDATE bocinfo_t SET lastaccount='%s',autologin=%s", sAccount.c_str(), m_checkAutoLogin->IsChecked() ? "true" : "false");
			}
			bodb_exec(sql);

			memset(sql, 0, sizeof(sql));
			sprintf(sql, "SELECT * FROM loginrecords_t WHERE account='%s'", sAccount.c_str());
			ret = bodb_exec(sql);

			// ??? test
			wxString sPasswordTemp = m_checkSavePassword->IsChecked() ? sPassword : "";
			if (ret < 1)
			{
				sprintf(sql, "INSERT INTO loginrecords_t(account,password) VALUES('%s','%s')", sAccount.c_str(), sPasswordTemp.c_str());
			}else
			{
				sprintf(sql, "UPDATE bocinfo_t SET account='%s',password='%s'", sAccount.c_str(), sPasswordTemp.c_str());
			}
			bodb_exec(sql);

		}

		wxDialog::EndModal(wxID_OK);
	}else
	{
		m_textAccount->SetFocus();
		m_textAccount->SetSelection(0, 0);
		m_btnLogin->Enable();
		//m_btnRegister->Enable();

		if (!m_cgcpReturned)
		{
			wxMessageBox(gLangText.textLoginTimeout(), gLangText.dlgLoginTitle(), wxOK | wxICON_ERROR, this);
		}else
		{
			switch (m_errorCode)
			{
			case 11:
			case 13:
				wxMessageBox(gLangText.textAccOrPwdError(), gLangText.dlgLoginTitle(), wxOK | wxICON_ERROR, this);
				break;
			default:
				wxString message;
				message.Format(_T("%s (%d)"), gLangText.textSystemError().c_str(), m_errorCode);
				wxMessageBox(message, gLangText.dlgLoginTitle(), wxOK | wxICON_ERROR, this);
				break;
			}
		}
	}
	*/
}

void CDlglogin::OnCancel(wxCommandEvent& WXUNUSED(event))
{
	bodb_exit();
	wxDialog::EndModal(wxID_CANCEL);
}

//void CDlglogin::OnRegister(wxCommandEvent& WXUNUSED(event))
//{
//
//}

void CDlglogin::OnClose(wxCloseEvent& event)
{
	bodb_exit();
	wxDialog::EndModal(wxID_CANCEL);
}

void CDlglogin::onUserLogined(CAccountInfo::pointer accountInfo)
{
	m_accountInfo = accountInfo;
	m_cgcpReturned = true;
}

void CDlglogin::onUserLoginError(long errorCode)
{
	m_errorCode = errorCode;
	m_cgcpReturned = true;
}