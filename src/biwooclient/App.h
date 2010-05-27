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

// App.h file here
#ifndef __App_h__
#define __App_h__

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
#include "wx/splitter.h"
#include "wx/notebook.h"
#include "wx/toolbook.h"
#include "wx/imaglist.h"
#include "wx/artprov.h"
#include "wx/cshelp.h"
#include "wx/utils.h"
#include "LeftWindow.h"
#include "RightWindow.h"
#include "incdef.h"
#include "MyTaskBarIcon.h"

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
	, public CbiwooHandler
{
private:

public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
    virtual int OnExit();

protected:
	int setAutorun(const char * programName, const char * filename) const;

	// CbiwooHandler handler
	virtual void onRegConfirm(void);
	virtual void onUserLogouted(CAccountInfo::pointer accountInfo, CbiwooHandler::LogoutType logoutType);
	virtual void onCompanyInfo(CCompanyInfo::pointer companyInfo);
	virtual void onCoGroupInfo(CCoGroupInfo::pointer cogroupInfo, bool isMyCoGroup);
	virtual void onCoGroupUser(CCoGroupInfo::pointer cogroupInfo, CUserInfoPointer userInfo, bool isMyCoGroupUser);

	virtual void onUserLogined(CCoGroupInfo::pointer cogroupInfo, CUserInfo::pointer userInfo);
	virtual void onUserLogouted(CCoGroupInfo::pointer cogroupInfo, CUserInfo::pointer userInfo);

	virtual void onDialogInvited(CDialogInfo::pointer dialogInfo, CUserInfo::pointer inviteUserInfo);

};

#include "MainPanel.h"
// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
private:
	wxSplitterWindow * m_splitter;
	bool m_showMsgIcon;
	//wxIcon m_iconTaskBar;
	wxString m_taskBarTooltip;

	//LeftWindow * m_left;
	//RightWindow * m_right;

	MyTaskBarIcon * m_taskBarIcon;
	wxTimer m_timer;

public:
    // ctor(s)
    MyFrame(const wxString& title, const wxSize& size);
	~MyFrame(void);

	void changeTaskBarIcon(const wxString & iconfilename, const wxString & tooltip = "");
	void setTaskBarIcon(const wxString & iconfilename, const wxString & tooltip);

protected:
    void ShowContextMenu(const wxPoint& pos);

    // event handlers (these functions should _not_ be virtual)
    void OnChangeAccount(wxCommandEvent& event);
	void OnInformationSetting(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnIconize(wxIconizeEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnCloseWindow(wxCloseEvent& event);
    void OnAbout(wxCommandEvent& event);

	void OnURL(wxTextUrlEvent& event);
	void OnTimer(wxTimerEvent & event);

private:
    // any class wishing to process wxWidgets events must use this macro
#if USE_CONTEXT_MENU
    void OnContextMenu(wxContextMenuEvent& event);
#else
    void OnRightUp(wxMouseEvent& event)
        { ShowContextMenu(event.GetPosition()); }
#endif

    DECLARE_EVENT_TABLE()


};

extern MyFrame * theFrame;

/*
// a frame showing the box sizer proportions
class MyProportionsFrame : public wxFrame
{
public:
    MyProportionsFrame(wxFrame *parent);

protected:
    void UpdateProportions();

    void OnProportionChanged(wxSpinEvent& event);
    void OnProportionUpdated(wxCommandEvent& event);

    wxSpinCtrl *m_spins[3]; // size can be changed without changing anything else
    wxSizer *m_sizer;
};

// a frame using flex sizers for layout
class MyFlexSizerFrame : public wxFrame
{
public:
    MyFlexSizerFrame(const wxChar *title, int x, int y );

private:
    void InitFlexSizer(wxFlexGridSizer *sizer, wxWindow* parent);
};


// a dialog using notebook sizer for layout
class MySizerDialog : public wxDialog
{
public:
    MySizerDialog(wxWindow *parent, const wxChar *title);
};


// a frame using wxGridBagSizer for layout
class MyGridBagSizerFrame : public wxFrame
{
public:
    MyGridBagSizerFrame(const wxChar *title, int x, int y );

    void OnHideBtn(wxCommandEvent&);
    void OnShowBtn(wxCommandEvent&);
    void OnMoveBtn(wxCommandEvent&);

private:
    wxGridBagSizer*     m_gbs;
    wxPanel*            m_panel;
    wxButton*           m_hideBtn;
    wxButton*           m_showBtn;
    wxTextCtrl*         m_hideTxt;

    wxButton*           m_moveBtn1;
    wxButton*           m_moveBtn2;
    wxGBPosition        m_lastPos;

    DECLARE_EVENT_TABLE()
};


// a frame for testing simple setting of "default size"
class MySimpleSizerFrame : public wxFrame
{
public:
    MySimpleSizerFrame(const wxChar *title, int x, int y );
    
    void OnSetSmallSize( wxCommandEvent &event);
    void OnSetBigSize( wxCommandEvent &event);
    
private:
    wxTextCtrl  *m_target;

    DECLARE_EVENT_TABLE()
};


// a frame for testing simple setting of a frame containing
// a sizer containing a panel containing a sizer containing
// controls
class MyNestedSizerFrame : public wxFrame
{
public:
    MyNestedSizerFrame(const wxChar *title, int x, int y );
    
    
private:
    wxTextCtrl  *m_target;
};

enum
{
    LAYOUT_TEST_SIZER = 101,
    LAYOUT_TEST_NB_SIZER,
    LAYOUT_TEST_GB_SIZER,
    LAYOUT_TEST_PROPORTIONS,
    LAYOUT_TEST_SET_MINIMAL,
    LAYOUT_TEST_NESTED,
    LAYOUT_QUIT = wxID_EXIT,
    LAYOUT_ABOUT = wxID_ABOUT
};
*/

#endif // __App_h__
