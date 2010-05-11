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

// DlgLogin.h file here
#ifndef __DlgLogin_h__
#define __DlgLogin_h__

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

/////////////////////////////////////////////////
//
//const int ID_REGISTER		= 2;

// A custom modeless dialog
class CDlglogin : public wxDialog
	, public CbiwooHandler
{
public:
    CDlglogin(wxWindow *parent, bool changeAccount);
    //CDlglogin(wxWindow *parent, CBiwooClient * biwoo);
	~CDlglogin(void);


	CAccountInfo::pointer accountInfo(void) const {return m_accountInfo;}

private:
	void loadDbInfo(void);
	bool doLogin(const std::string & sAccount, const std::string & sPassword, bool autoLogin);

    void OnLogin(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    //void OnRegister(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
	void OnTimer(wxTimerEvent & event);

    DECLARE_EVENT_TABLE()

	// CbiwooHandler
	virtual void onUserLogined(CAccountInfo::pointer accountInfo);
	virtual void onUserLoginError(long errorCode);

private:
	wxButton * m_btnLogin;
	//wxButton * m_btnRegister;
	wxTextCtrl * m_textAccount;
	wxTextCtrl * m_textPassword;
	wxCheckBox * m_checkSavePassword;
	wxCheckBox * m_checkAutoLogin;
	wxTimer m_timer;

	//CBiwooClient * m_biwoo;
	CAccountInfo::pointer m_accountInfo;
	CLockMap<std::string, CUserInfo::pointer> m_records;
	long m_errorCode;
	bool m_cgcpReturned;
	bool m_changeAccount;
};


#endif // __DlgLogin_h__
