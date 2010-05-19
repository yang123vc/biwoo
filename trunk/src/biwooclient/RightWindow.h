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

// RightWindow.h file here
#ifndef __RightWindow_h__
#define __RightWindow_h__

//#include <libP2PAV/libP2PAV.h>

#include "wx/wxprec.h"
#include "wx/splitter.h"
#include "wx/richtext/richtextctrl.h"
#include "wx/richtext/richtextstyles.h"
#include "wx/richtext/richtextxml.h"
#include "wx/richtext/richtexthtml.h"
#include "wx/richtext/richtextformatdlg.h"
#include "wx/richtext/richtextsymboldlg.h"
#include "wx/richtext/richtextstyledlg.h"
#include "wx/richtext/richtextprint.h"
#include "../biwooinfo/accconversation.h"
#include "VideoChatWindow.h"
#include "GroupChatWindow.h"
#include "incdef.h"

struct RTPOS
{
	long from;
	long to;
};

class CCommandInfo
{
public:
	enum CommandType
	{
		CT_None			= 0x1
		, CT_Accept
		, CT_SaveAs
		, CT_Reject
		, CT_Cancel
		, CT_OpenFile
		, CT_OpenFolder
	};

	typedef boost::shared_ptr<CCommandInfo> pointer;
	static CCommandInfo::pointer create(const std::string & url, CommandType command, CConversationInfo::pointer conversationInfo, const RTPOS & commandPos)
	{
		return CCommandInfo::pointer(new CCommandInfo(url, command, conversationInfo, commandPos));
	}

	const std::string & url(void) const {return m_url;}
	void command(CommandType newv) {m_command = newv;}
	CommandType command(void) const {return m_command;}
	CConversationInfo::pointer conversationInfo(void) const {return m_conversationInfo;}

	const RTPOS & commandPos(void) const {return m_commandPos;}
	//const RTPOS & messagePos(void) const {return m_messagePos;}

	void offset(int offset, bool from = true)
	{
		if (from)
			m_commandPos.from += offset;
		m_commandPos.to += offset;
	}
public:
	CCommandInfo(const std::string & url, CommandType command, CConversationInfo::pointer conversationInfo, const RTPOS & commandPos)
		: m_url(url), m_command(command), m_conversationInfo(conversationInfo)
		, m_commandPos(commandPos)//, m_messagePos(messagePos)
	{
		BOOST_ASSERT (m_conversationInfo.get() != NULL);
	}

private:
	std::string m_url;
	CommandType m_command;
	CConversationInfo::pointer m_conversationInfo;
	RTPOS m_commandPos;
	//RTPOS m_messagePos;

};

class CMessagePos
{
public:
	typedef boost::shared_ptr<CMessagePos> pointer;
	static CMessagePos::pointer create(long mid, const RTPOS & messagePos, const RTPOS & filenamePos)
	{
		return CMessagePos::pointer(new CMessagePos(mid, messagePos, filenamePos));
	}

	long mid(void) const {return m_mid;}
	const RTPOS & messagePos(void) const {return m_messagePos;}
	const RTPOS & filenamePos(void) const {return m_filenamePos;}

	void cmd1(CCommandInfo::pointer newv) {m_cmd1 = newv;}
	CCommandInfo::pointer cmd1(void) const {return m_cmd1;}

	void cmd2(CCommandInfo::pointer newv) {m_cmd2 = newv;}
	CCommandInfo::pointer cmd2(void) const {return m_cmd2;}

	void filepath(const std::string & newv) {m_filepath = newv;}
	const std::string & filepath(void) const {return m_filepath;}
	void filename(const std::string & newv) {m_filename = newv;}
	const std::string & filename(void) const {return m_filename;}

	void offset(int offset, bool from = true)
	{
		m_messagePos.to += offset;
		m_filenamePos.to += offset;
		if (m_cmd1.get() != NULL)
			m_cmd1->offset(offset, from);
		if (m_cmd2.get() != NULL)
			m_cmd2->offset(offset, from);
	}

public:
	CMessagePos(long mid, const RTPOS & msgPos, const RTPOS & filePos)
		: m_mid(mid), m_messagePos(msgPos), m_filenamePos(filePos)
		, m_filepath(""), m_filename("")
	{
	}

private:
	long m_mid;
	RTPOS m_messagePos;
	RTPOS m_filenamePos;

	CCommandInfo::pointer m_cmd1;
	CCommandInfo::pointer m_cmd2;

	std::string m_filepath;
	std::string m_filename;
};

class RightWindow
	: public wxPanel
{
public:
	RightWindow(wxWindow * parent);
	~RightWindow(void);

	void reset(void);
	void setCurrentSelect(CFromInfo::pointer fromInfo);
	bool isCurrentSelect(CFromInfo::pointer fromInfo) const;

	CFromInfo::pointer getCurrentSelect(void) const {return m_curFromInfo;}
	//void showHistory(CAccountConversation::pointer account);

	void invitedUser(CUserInfo::pointer inviteUserInfo);
	void clearMessage(void);
	void addMessage(CConversationInfo::pointer conversationInfo);

	wxString getParentCoGroup(CFromInfo::pointer fromInfo) const;

	void onUserLogined(CCoGroupInfo::pointer cogroupInfo, CUserInfo::pointer userInfo);
	void onUserLogouted(CCoGroupInfo::pointer cogroupInfo, CUserInfo::pointer userInfo);

protected:
	void addMsgDate(time_t msgtime);
	void DoAsyncExec(const wxString & cmd);
	bool FileExec(const wxString & filename );

	wxString getFilesize(unsigned long filesize) const;

	void createUserInfoPanel(wxWindow * parent);
	void createMsgPanel(wxWindow * parent);
	void createChatPanel(wxWindow * parent);

private:
	wxSplitterWindow * m_splitter;
	bool m_showVideoPanel;
	VideoChatWindow * m_panelVideo;
	GroupChatWindow * m_panelGroup;
	wxPanel * m_panelUserInfo;
	wxStaticText * m_staName;
	wxStaticText * m_staUserEMail;
	wxStaticText * m_staUserPhone;
	wxStaticText * m_staUserDepartment;

	wxPanel * m_panelMsg;
	wxButton * m_btnSendFile;
	wxButton * m_btnVideoCall;
	wxButton * m_btnAddUser;
	wxButton * m_btnQuitDialog;
	wxRichTextCtrl * m_richHistory;

	wxPanel * m_panelChat;
	wxRichTextCtrl * m_richTextCtrl;
	wxButton * m_btnSendMsg;

	wxBitmap m_bitmapUI;
	wxStaticBitmap * m_staLocal;
	wxStaticBitmap * m_staRemove;

	wxBoxSizer * m_sizerUserinfo;

	void OnSize(wxSizeEvent& event);
    void OnSendMsg(wxCommandEvent& event);
    void OnSendFile(wxCommandEvent& event);
    void OnVideoCall(wxCommandEvent& event);
    void OnAddUser(wxCommandEvent& event);
    void OnQuitDialog(wxCommandEvent& event);
	void OnTimer(wxTimerEvent & event);
    void OnURL(wxTextUrlEvent& event);

	DECLARE_EVENT_TABLE()

	//wxTimer m_timer;
	int m_currentShowDay;	// YYYYMMDD
	CFromInfo::pointer m_curFromInfo;

	CLockMap<std::string, CCommandInfo::pointer> m_commands;
	CLockMap<long, CMessagePos::pointer> m_msgpos;
	CLockMap<long, CMessagePos::pointer> m_recvpos;

	wxRichTextAttr m_dateStyle;
	wxRichTextAttr m_ownerStyle;
	wxRichTextAttr m_blueStyle;
	wxRichTextAttr m_urlStyle;
	wxRichTextAttr m_noneUrlStyle;
};

extern RightWindow *	gRightWindow;

#endif // __RightWindow_h__
