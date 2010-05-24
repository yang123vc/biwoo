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
#include "App.h"
#include "DlgLogin.h"

MyFrame * theFrame = NULL;

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_EXIT,  MyFrame::OnQuit)
    EVT_MENU(ID_Menu_ChangeAccount,  MyFrame::OnChangeAccount)
    EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
	EVT_CLOSE(MyFrame::OnCloseWindow) 
	EVT_ICONIZE(OnIconize)
	//EVT_SIZE(MyFrame::OnSize) 

#if USE_CONTEXT_MENU
    EVT_CONTEXT_MENU(MyFrame::OnContextMenu)
#else
    EVT_RIGHT_UP(MyFrame::OnRightUp)
#endif
    EVT_TEXT_URL(wxID_ANY, MyFrame::OnURL)
	EVT_TIMER(wxID_ANY, MyFrame::OnTimer)

END_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
	gAppHandler = this;

	wxString sFullModleName = ::wxGetFullModuleName();
	wxString sAppName = this->GetAppName();
	size_t nFind = sFullModleName.rfind(sAppName);
	gAppPath = sFullModleName.substr(0, nFind-1).c_str();

	setAutorun("Biwoo", sFullModleName.c_str());

	wxString xmlFile(gAppPath);
	xmlFile.Append(wxT("/default/setting.xml"));
	gSetting.load(xmlFile.c_str());

	setlocale(LC_ALL, gSetting.locale().c_str());
	//setlocale(LC_ALL,"Chinese-simplified");
	//std::locale::global(std::locale("Chinese-simplified"));

	CCgcAddress serverAddress(gSetting.address(), CCgcAddress::ST_UDP);
	//CCgcAddress fileserverAddress(gSetting.fileserver(), CCgcAddress::ST_UDP);
	//CCgcAddress rtpserverAddress(gSetting.p2prtp(), CCgcAddress::ST_RTP);
	//CCgcAddress udpserverAddress(gSetting.p2pudp(), CCgcAddress::ST_UDP);

	if (!m_biwoo.start(serverAddress, (CbiwooHandler*)this))
	{
		//m_biwoo.stop();
		//return false;
	}

	xmlFile = gAppPath.c_str();
	xmlFile.Append(wxT("/default/"));
	xmlFile.Append(gSetting.langtext());
	gLangText.load(xmlFile.c_str());

    if ( !wxApp::OnInit() )
	{
		m_biwoo.stop();
        return false;
	}

	CDlglogin dlgLogin(NULL, false);
	int ret = dlgLogin.ShowModal();
	if (ret != wxID_OK)
	{
		m_biwoo.stop();
		wxApp::OnExit();
		return false;
	}
	gMyAccount = dlgLogin.accountInfo();
	BOOST_ASSERT (gMyAccount.get() != 0);

    // create the main application window
    MyFrame * frame = new MyFrame(_T("BIWOO App"), wxSize(700, 450));
	m_biwoo.setHandler(gAppHandler);
	m_biwoo.accountLoad();

	wxString title = wxString::Format("biwoo - %s(%s)", gMyAccount->getUserinfo()->getUserName().c_str(), gMyAccount->getUserinfo()->getAccount().c_str());
	frame->SetTitle(title);
	frame->setTaskBarIcon(wxT("mainframe.ico"), title);

#if defined(__WXMOTIF__)
    int width, height;
    frame->GetSize(& width, & height);
    frame->SetSize(wxDefaultCoord, wxDefaultCoord, width, height);
#endif

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
	frame->SetMinSize(wxSize(700, 450));
    frame->Show(true);

    //SetTopWindow(frame);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}
int MyApp::OnExit()
{
	gMyAccount.reset();
	gAppHandler = NULL;
	m_biwoo.stop();
	return wxApp::OnExit();
}

int MyApp::setAutorun(const char * programName, const char * filename) const
{
	// Set auto run
#ifdef WIN32
	char     *     strRegName[5]     ={"SOFTWARE","Microsoft","Windows","CurrentVersion","Run"};    
	HKEY                 hKey     =     HKEY_LOCAL_MACHINE;    
	HKEY                 hSubKey;    
	int                   indx     =     0;    
	long                 lRC;    
	char                 svBuffer[256];    
	DWORD               dwSize     =     256;    
	BOOL                 bReg     =     FALSE;    
	do    
	{    
		lRC     =     RegOpenKeyEx(hKey,     strRegName[indx],     0,    
			KEY_READ     |     KEY_WRITE,     &hSubKey)     ;    
		if(lRC     !=     ERROR_SUCCESS)    
		{    
			RegCloseKey(hKey);    
			return     -1;    
		}    
		hKey     =     hSubKey;    
	} while (++indx < 5);
	lRC     =     RegQueryValueEx(hKey,    programName,     NULL,     NULL,    
		(BYTE     *)svBuffer,     &dwSize);    
	if     (lRC     !=     ERROR_SUCCESS)    
	{    
		lRC     =     RegSetValueEx     (hKey,     programName,     0,     REG_SZ,    
			(BYTE     *)filename,strlen(filename));    
		bReg     =     FALSE;    
	}    
	else    
		bReg     =     TRUE;     
#endif

	return 0;
}

void MyApp::onRegConfirm(void)
{
	assert (gMyAccount.get() != NULL);

	if (gMyCoGroup != NULL)
		gMyCoGroup->onRegConfirm();
}

void MyApp::onUserLogouted(CAccountInfo::pointer accountInfo, CbiwooHandler::LogoutType logoutType)
{
	assert (accountInfo.get() != NULL);
	gMyAccount.reset();

	switch (logoutType)
	{
	case CbiwooHandler::ServerQuit:
		wxMessageBox(gLangText.textServerQuitTip(), _T("biwoo"), wxOK | wxICON_INFORMATION, NULL);
		break;
	case CbiwooHandler::LT_LoginAnotherPlace:
		wxMessageBox(gLangText.textLoginAnotherPlaceTip(), _T("biwoo"), wxOK | wxICON_INFORMATION, NULL);
		break;
	default:
		{
			if (gMyCompany != NULL)
				gMyCompany->onUserLogouted(accountInfo, logoutType);
			if (gMyCoGroup != NULL)
				gMyCoGroup->onUserLogouted(accountInfo, logoutType);

			return;
		}break;
	}
	if (theFrame != NULL)
	{
		theFrame->Destroy();
	}
}

void MyApp::onDialogInvited(CDialogInfo::pointer dialogInfo, CUserInfo::pointer inviteUserInfo)
{
	assert (dialogInfo.get() != NULL);
	assert (inviteUserInfo.get() != NULL);

	if (gMyCoGroup != NULL)
		gMyCoGroup->onDialogInvited(dialogInfo, inviteUserInfo);
}

void MyApp::onCompanyInfo(CCompanyInfo::pointer companyInfo)
{
	if (gMyCompany != NULL)
		gMyCompany->onCompanyInfo(companyInfo);
	if (gMyCoGroup != NULL)
		gMyCoGroup->onCompanyInfo(companyInfo);
}
	
void MyApp::onCoGroupInfo(CCoGroupInfo::pointer cogroupInfo, bool isMyCoGroup)
{
	if (gMyCompany != NULL)
		gMyCompany->onCoGroupInfo(cogroupInfo, isMyCoGroup);
	if (isMyCoGroup && gMyCoGroup != NULL)
		gMyCoGroup->onCoGroupInfo(cogroupInfo, isMyCoGroup);

}
void MyApp::onCoGroupUser(CCoGroupInfo::pointer cogroupInfo, CUserInfoPointer userInfo, bool isMyCoGroupUser)
{
	if (gMyCompany != NULL)
		gMyCompany->onCoGroupUser(cogroupInfo, userInfo, isMyCoGroupUser);
	if (isMyCoGroupUser && gMyCoGroup != NULL)
		gMyCoGroup->onCoGroupUser(cogroupInfo, userInfo, isMyCoGroupUser);
}

void MyApp::onUserLogined(CCoGroupInfo::pointer cogroupInfo, CUserInfo::pointer userInfo)
{
	if (gMyCompany != NULL)
		gMyCompany->onUserLogined(cogroupInfo, userInfo);
	if (gMyCoGroup != NULL)
		gMyCoGroup->onUserLogined(cogroupInfo, userInfo);
}

void MyApp::onUserLogouted(CCoGroupInfo::pointer cogroupInfo, CUserInfo::pointer userInfo)
{
	if (gMyCompany != NULL)
		gMyCompany->onUserLogouted(cogroupInfo, userInfo);
	if (gMyCoGroup != NULL)
		gMyCoGroup->onUserLogouted(cogroupInfo, userInfo);
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
MyFrame::MyFrame(const wxString& title, const wxSize& size)
       : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, size)
	   , m_splitter(NULL)
	   , m_showMsgIcon(false)
	   , m_taskBarIcon(NULL)
{
	wxString sTemp(gAppPath);
	sTemp.Append(wxT("/res/mainframe.ico"));
	wxIcon icon(sTemp, wxBITMAP_TYPE_ICO);

    // set the frame icon
    SetIcon(icon);

#if wxUSE_MENUS
    // create a menu bar
    wxMenu *fileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    //helpMenu->Append(wxID_ABOUT, _T("&About...\tF1"), _T("Show about dialog"));

    fileMenu->Append(ID_Menu_ChangeAccount, gLangText.menuChangeAccText(), gLangText.menuChangeAccHelp());
	fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, gLangText.menuExitText(), gLangText.menuExitHelp());

 	helpMenu->Append(wxID_ABOUT, gLangText.menuAboutText(), gLangText.menuAboutHelp());

   // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, gLangText.menuFile());
    menuBar->Append(helpMenu, gLangText.menuHelp());

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
    SetStatusText(gLangText.textWelcome());
#endif // wxUSE_STATUSBAR

    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, GetClientSize(), wxSP_NOBORDER);

    //m_left = new LeftWindow(m_splitter);
    RelationshipWindow * relationship = new RelationshipWindow(m_splitter);
	RightWindow * right = new RightWindow(m_splitter);
	right->SetBackgroundColour(*wxWHITE);
	m_splitter->SetMinimumPaneSize(60);
	m_splitter->SplitVertically(relationship, right, 180);

	//m_richTextCtrl = new wxRichTextCtrl(m_splitter, ID_RICHTEXT_CTRL, wxEmptyString, wxDefaultPosition, wxSize(200, 200), wxVSCROLL|wxHSCROLL|wxNO_BORDER|wxWANTS_CHARS);
	//m_splitter->SplitVertically(m_left, m_richTextCtrl, 200);
	m_timer.SetOwner(this);
	//m_timer.Start(1000);

    m_splitter->UpdateSize();
	theFrame = this;
}

MyFrame::~MyFrame(void)
{
	m_timer.Stop();
	theFrame = NULL;
	delete m_splitter;

	if (m_taskBarIcon != NULL)
		delete m_taskBarIcon;
}

void MyFrame::OnURL(wxTextUrlEvent& event)
{
    wxMessageBox(event.GetString());
}


void MyFrame::OnTimer(wxTimerEvent & event)
{
	if (m_taskBarIcon != NULL)
	{
		//if (m_biwoo.hasUnread())
		//{
		//	if (m_showMsgIcon) return;

		//	wxString sTemp(gAppPath.c_str());
		//	sTemp.Append(wxT("/res/unread.ico"));

		//	wxIcon icon(sTemp, wxBITMAP_TYPE_ICO);
		//	m_taskBarIcon->SetIcon(icon, m_taskBarTooltip);
		//}else
		//{
		//	if (!m_showMsgIcon) return;

		//	m_taskBarIcon->SetIcon(m_iconTaskBar, m_taskBarTooltip);
		//}
		//m_showMsgIcon = !m_showMsgIcon;

//		// Set another icon.
//		if (m_taskBarIcon->IsIconInstalled())
//		{
//			m_taskBarIcon->RemoveIcon();
//#ifdef WIN32
//			Sleep(100);
//#else
//			usleep(100000);
//#endif
//		}

		//m_taskBarIcon->SetIcon(m_iconTaskBar, m_taskBarTooltip);
	}
}

void MyFrame::changeTaskBarIcon(const wxString & iconfilename, const wxString & tooltip)
{
	if (m_taskBarIcon != NULL)
	{
		wxString sTooltip = tooltip.empty() ? m_taskBarTooltip : tooltip;

		wxString sTemp(gAppPath.c_str());
		sTemp.Append(wxT("/res/"));
		sTemp.Append(iconfilename);

		wxIcon iconTaskBar(sTemp, wxBITMAP_TYPE_ICO);

		if (!m_taskBarIcon->SetIcon(iconTaskBar, sTooltip))
		{
			wxMessageBox(wxT("Could not set icon."));
		}
	}
}

void MyFrame::setTaskBarIcon(const wxString & iconfilename, const wxString & tooltip)
{
	wxString sTemp(gAppPath.c_str());
	sTemp.Append(wxT("/res/"));
	sTemp.Append(iconfilename);

	wxIcon iconTaskBar(sTemp, wxBITMAP_TYPE_ICO);
	m_taskBarTooltip = tooltip;

	if (m_taskBarIcon == NULL)
	{
		m_taskBarIcon = new MyTaskBarIcon(this);
	}

	if (!m_taskBarIcon->SetIcon(iconTaskBar, m_taskBarTooltip))
	{
		wxMessageBox(wxT("Could not set icon."));
	}
}

void MyFrame::OnSize(wxSizeEvent& event)
{
	if (m_splitter)
		m_splitter->SetSize(this->GetSize());
	if (event.GetSize().GetHeight() == 0 && event.GetSize().GetWidth() == 0)
	{
		this->Show(false);
	}

}

// event handlers

static bool bQuitApp = false;
void MyFrame::OnCloseWindow(wxCloseEvent& event)
{
	if (bQuitApp)
	{
		this->Destroy();
	}else
	{
		event.Veto();
		this->Show(false);
	}
}

void MyFrame::OnChangeAccount(wxCommandEvent& WXUNUSED(event))
{
	if (wxMessageBox(gLangText.menuChangeAccQuestion(), wxT("biwoo"), wxICON_QUESTION | wxYES_NO, this) == wxYES)
	{
		this->Show(false);
		m_biwoo.accountUnRegister();

		short i=0;
		while (++i < 50)
		{
			if (gMyAccount.get() == NULL)
				break;
#ifdef WIN32
			Sleep(100);
#else
			usleep(100000);
#endif
		}
		
		CDlglogin dlgLogin(NULL, true);
		int ret = dlgLogin.ShowModal();
		if (ret != wxID_OK)
		{
			bQuitApp = true;
			Close(true);
			return;
		}

		gMyAccount = dlgLogin.accountInfo();
		BOOST_ASSERT (gMyAccount.get() != 0);

		m_biwoo.setHandler(gAppHandler);
		m_biwoo.accountLoad();

		wxString title = wxString::Format("biwoo - %s(%s)", gMyAccount->getUserinfo()->getUserName().c_str(), gMyAccount->getUserinfo()->getAccount().c_str());
		SetTitle(title);
		setTaskBarIcon(wxT("mainframe.ico"), title);
		this->Show(true);
	}
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	if (wxMessageBox(gLangText.menuExitQuestion(), wxT("biwoo"), wxICON_QUESTION | wxYES_NO, this) == wxYES)
	{
		bQuitApp = true;
		Close(true);
	}
}


void MyFrame::OnIconize(wxIconizeEvent& event)
{
	this->Show(false);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format(
                    _T("Welcome to biwoo client!\n")
                    _T("\n")
                    _T("This is the open source enterprise communication application \n")
                    _T("running under %s.\n")
                    _T("\n")
					_T("Version 1.0.2\n")
					_T("Copyright (C)2009-2010 akee.yang@gmail.com\n")
					_T("All rights reserved.\n"),
                    wxGetOsDescription().c_str()
                 ),
                 _T("About biwoo client"),
                 wxOK | wxICON_INFORMATION,
                 this);
}

void MyFrame::ShowContextMenu(const wxPoint& pos)
{
    wxMenu menu;

	menu.Append(wxID_ABOUT, gLangText.menuAboutText(), gLangText.menuAboutHelp());
    //menu.Append(Menu_Popup_Submenu, _T("&Submenu"), CreateDummyMenu(NULL));
    menu.AppendSeparator();
    menu.Append(wxID_EXIT, gLangText.menuExitText(), gLangText.menuExitHelp());

    //menu.Delete(Menu_Popup_ToBeDeleted);
    //menu.Check(Menu_Popup_ToBeChecked, true);
    //menu.Enable(Menu_Popup_ToBeGreyed, false);

    PopupMenu(&menu, pos.x, pos.y);
}

#if USE_CONTEXT_MENU
void MyFrame::OnContextMenu(wxContextMenuEvent& event)
{
    wxPoint point = event.GetPosition();
    // If from keyboard
    if (point.x == -1 && point.y == -1) {
        wxSize size = GetSize();
        point.x = size.x / 2;
        point.y = size.y / 2;
    } else {
        point = ScreenToClient(point);
    }
    ShowContextMenu(point);
}
#endif
