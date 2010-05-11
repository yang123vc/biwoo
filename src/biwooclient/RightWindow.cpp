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
#include "RightWindow.h"
#include <wx/mimetype.h>
#include <wx/process.h>
#include "wx/artprov.h"
#include <sys/timeb.h>
#include "DlgAddUser.h"


#ifdef WIN32
#include <shellapi.h>
#endif // WIN32

RightWindow *	gRightWindow = NULL;

//BEGIN_EVENT_TABLE(RightScrolledWind+zow, wxScrolledWindow)
BEGIN_EVENT_TABLE(RightWindow, wxPanel)

EVT_SIZE    (           RightWindow::OnSize)

EVT_BUTTON(Btn_SendMsg, RightWindow::OnSendMsg)
EVT_BUTTON(Btn_SendFile, RightWindow::OnSendFile)
EVT_BUTTON(Btn_VideoCall, RightWindow::OnVideoCall)
EVT_BUTTON(Btn_AddUser, RightWindow::OnAddUser)
EVT_BUTTON(Btn_QuitDialog, RightWindow::OnQuitDialog)
EVT_TIMER(wxID_ANY, RightWindow::OnTimer)
    EVT_TEXT_URL(wxID_ANY, RightWindow::OnURL)

END_EVENT_TABLE()

RightWindow::RightWindow(wxWindow * parent)
: wxPanel(parent)
, m_splitter(NULL), m_panelUserInfo(NULL), m_panelMsg(NULL), m_panelChat(NULL)
, m_showVideoPanel(false), m_currentShowDay(0)

{
	wxString filename(gAppPath);
	filename.append("/res/biwooman.bmp");
	m_bitmapUI.LoadFile(filename, wxBITMAP_TYPE_BMP);

	m_dateStyle.SetTextColour(*wxLIGHT_GREY);
	m_dateStyle.SetFontUnderlined(false);
	m_ownerStyle.SetTextColour(*wxGREEN);
	m_ownerStyle.SetFontUnderlined(false);
	m_blueStyle.SetTextColour(*wxBLUE);
	m_blueStyle.SetFontUnderlined(false);
	m_urlStyle.SetTextColour(*wxBLUE);
	m_urlStyle.SetFontUnderlined(true);
	m_noneUrlStyle.SetTextColour(*wxBLACK);
	m_noneUrlStyle.SetFontUnderlined(false);

    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, GetClientSize(), wxSP_NOBORDER);
    wxSplitterWindow * splitter2 = new wxSplitterWindow(m_splitter, wxID_ANY, wxDefaultPosition, GetClientSize(), wxSP_NOBORDER);

	createUserInfoPanel(m_splitter);
	m_panelGroup = new GroupChatWindow(m_splitter);
	m_panelGroup->Show(false);
	m_panelVideo = new VideoChatWindow(m_splitter);
	m_panelVideo->Show(false);
	createMsgPanel(splitter2);
	createChatPanel(splitter2);

	splitter2->SetMinimumPaneSize(60);
	splitter2->SplitHorizontally(m_panelMsg, m_panelChat, 60);

	m_splitter->SetMinimumPaneSize(60);
	m_splitter->SplitHorizontally(m_panelUserInfo, splitter2, 60);
	splitter2->SetSashPosition(500);

	gRightWindow = this;
	//m_timer.SetOwner(this);
	//m_timer.Start(200);
}

RightWindow::~RightWindow(void)
{
	delete m_staName;
	delete m_staUserEMail;
	delete m_staUserPhone;
	delete m_staUserDepartment;

	delete m_btnSendFile;
	delete m_btnVideoCall;
	delete m_btnAddUser;
	//delete m_btnQuitDialog;
	delete m_richHistory;

	delete m_richTextCtrl;
	delete m_btnSendMsg;

	if (m_splitter->GetWindow1() == m_panelUserInfo)
	{
		delete m_panelVideo;
		delete m_panelGroup;
	}else if (m_splitter->GetWindow1() == m_panelVideo)
	{
		delete m_panelUserInfo;
		delete m_panelGroup;
	}else
	{
		delete m_panelVideo;
		delete m_panelUserInfo;
	}

	delete m_splitter;

	m_commands.clear();
	m_msgpos.clear();
	m_recvpos.clear();
	gRightWindow = NULL;
	//m_timer.Stop();
}

void RightWindow::createUserInfoPanel(wxWindow * parent)
{
	assert (parent != NULL);

	if (m_panelUserInfo == NULL)
	{
		m_panelUserInfo = new wxPanel(parent, wxID_ANY);
		m_sizerUserinfo = new wxBoxSizer(wxHORIZONTAL);
		//wxStaticBitmap * staUserImage = new wxStaticBitmap(this, wxID_ANY, m_bitmapUI, wxDefaultPosition, wxSize(128, 128));
		wxBoxSizer * sizerUserDetail = new wxBoxSizer(wxVERTICAL);
		m_staName = new wxStaticText(m_panelUserInfo, wxID_ANY, wxT(""));
		
		wxBoxSizer * sizerUserDetail1 = new wxBoxSizer(wxHORIZONTAL);
		m_staUserEMail = new wxStaticText(m_panelUserInfo, wxID_ANY, wxT(""));
		m_staUserPhone = new wxStaticText(m_panelUserInfo, wxID_ANY, wxT(""));
		sizerUserDetail1->Add(m_staUserEMail, 1, wxALL|wxALIGN_CENTER, 5);
		sizerUserDetail1->Add(m_staUserPhone, 1, wxALL|wxALIGN_CENTER, 5);

		m_staUserDepartment = new wxStaticText(m_panelUserInfo, wxID_ANY, wxT(""));

		sizerUserDetail->Add(m_staName, 0, wxALL|wxALIGN_LEFT, 5);
		sizerUserDetail->Add(sizerUserDetail1, 1, wxALL|wxALIGN_LEFT, 0);
		sizerUserDetail->Add(m_staUserDepartment, 0, wxALL|wxALIGN_LEFT, 5);
		// ???m_sizerUserinfo->Add(staUserImage, 1, wxALL|wxEXPAND|wxALIGN_CENTER, 5);
		m_sizerUserinfo->Add(sizerUserDetail, 1, wxALL|wxALIGN_CENTER, 5);

		m_panelUserInfo->SetSizer(m_sizerUserinfo);
		m_sizerUserinfo->SetSizeHints(m_panelUserInfo);
		m_sizerUserinfo->Fit(m_panelUserInfo);
	}
}

void RightWindow::createMsgPanel(wxWindow * parent)
{
	assert (parent != NULL);

	if (m_panelMsg == NULL)
	{
		wxBoxSizer *sizerTop = new wxBoxSizer(wxVERTICAL);
		m_panelMsg = new wxPanel(parent, wxID_ANY);

		wxBoxSizer * sizerButtons = new wxBoxSizer(wxHORIZONTAL);
		m_btnSendFile = new wxButton(m_panelMsg, Btn_SendFile, gLangText.btnSendFileText());
		m_btnSendFile->SetToolTip(gLangText.btnSendFileHelp());
		m_btnVideoCall = new wxButton(m_panelMsg, Btn_VideoCall, gLangText.btnVideoCallText());
		m_btnVideoCall->SetToolTip(gLangText.btnVideoCallHelp());
		m_btnAddUser = new wxButton(m_panelMsg, Btn_AddUser, gLangText.btnAddUserText());
		m_btnAddUser->SetToolTip(gLangText.btnAddUserHelp());
		//m_btnQuitDialog = new wxButton(m_panelMsg, Btn_QuitDialog, gLangText.btnQuitDialogText());
		//m_btnQuitDialog->SetToolTip(gLangText.btnQuitDialogHelp());
		sizerButtons->Add(m_btnSendFile, 0, wxALL|wxALIGN_LEFT, 3);
		sizerButtons->Add(m_btnVideoCall, 0, wxALL|wxALIGN_LEFT, 3);
		sizerButtons->Add(m_btnAddUser, 0, wxALL|wxALIGN_LEFT, 3);
		//sizerButtons->Add(m_btnQuitDialog, 0, wxALL|wxALIGN_LEFT, 3);

		wxBoxSizer * sizerHistory = new wxBoxSizer(wxHORIZONTAL);
		m_richHistory = new wxRichTextCtrl(m_panelMsg, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL|wxNO_BORDER|wxRE_READONLY|wxWANTS_CHARS);
		sizerHistory->Add(m_richHistory, 1, wxALL|wxEXPAND|wxALIGN_CENTER, 5);
		wxFont fontHistory(9, wxROMAN, wxNORMAL, wxNORMAL);
		m_richHistory->SetFont(fontHistory);

		sizerTop->Add(sizerButtons, 0, wxALL|wxALIGN_TOP, 5);
		sizerTop->Add(sizerHistory, 1, wxALL|wxEXPAND|wxALIGN_CENTER, 5);

		m_panelMsg->SetSizer(sizerTop);
		sizerTop->SetSizeHints(m_panelMsg);
		sizerTop->Fit(m_panelMsg);
	}

}

void RightWindow::createChatPanel(wxWindow * parent)
{
	assert (parent != NULL);
	if (m_panelChat == NULL)
	{
		wxBoxSizer *sizerTop = new wxBoxSizer(wxVERTICAL);
		m_panelChat = new wxPanel(parent, wxID_ANY);

		wxBoxSizer * sizerInput = new wxBoxSizer(wxHORIZONTAL);
		m_richTextCtrl = new wxRichTextCtrl(m_panelChat, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL);
		wxFont fontInput(9, wxROMAN, wxNORMAL, wxNORMAL);
		m_richTextCtrl->SetFont(fontInput);
		m_richTextCtrl->SetFocus();

		m_btnSendMsg = new wxButton(m_panelChat, Btn_SendMsg, gLangText.btnSendMsgText());
		m_btnSendMsg->SetToolTip(gLangText.btnSendMsgHelp());
		sizerInput->Add(m_richTextCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER, 3);
		sizerInput->Add(m_btnSendMsg, 0, wxALL|wxALIGN_RIGHT, 3);

		sizerTop->Add(sizerInput, 1, wxALL|wxEXPAND|wxALIGN_CENTER, 5 );

		m_panelChat->SetSizer(sizerTop);
		sizerTop->SetSizeHints(m_panelChat);
		sizerTop->Fit(m_panelChat);
	}
}

void RightWindow::OnTimer(wxTimerEvent & event)
{
	//if (m_curUserInfo.get() == NULL) return;
}

void RightWindow::DoAsyncExec(const wxString & cmd)
{
    wxExecute(cmd);

	//wxProcess *process = new wxProcess(this);
    //long m_pidLast = wxExecute(cmd, wxEXEC_ASYNC, process);
    //if ( !m_pidLast )
    //{
    //    //wxLogError( _T("Execution of '%s' failed."), cmd.c_str() );

    //    delete process;
    //}
    //else
    //{
    //    wxLogStatus( _T("Process %ld (%s) launched."),
    //        m_pidLast, cmd.c_str() );
    //}
}

bool RightWindow::FileExec(const wxString & filename )
{
    //static wxString s_filename;
    if ( filename.empty() )
        return false;

    wxString ext = filename.AfterLast(_T('.'));
    wxFileType *ft = wxTheMimeTypesManager->GetFileTypeFromExtension(ext);
    if ( !ft )
    {
        wxLogError(_T("Impossible to determine the file type for extension '%s'"),
            ext.c_str());
        return false;
    }

    wxString cmd;

//#ifdef WIN32
//	wxIconLocation iconLocation;
//
//	bool ok = ft->GetIcon(&iconLocation);
//    delete ft;
//
//	if (!ok)
//	{
//		return false;
//	}
//	cmd = wxString::Format("\"%s\" \"%s\"", iconLocation.GetFileName().c_str(), filename.c_str());
//#else
	//wxString mimeType;
	//ft->GetMimeType(&mimeType);
	//bool ok = ft->GetOpenCommand(&cmd, wxFileType::MessageParameters(filename, mimeType));

    bool ok = ft->GetOpenCommand(&cmd, wxFileType::MessageParameters(filename));
    delete ft;
    if ( !ok )
    {
        wxLogError(_T("Impossible to find out how to open files of extension '%s'"),
            ext.c_str());
        return false;
    }
//#endif

    DoAsyncExec(cmd);
    return true;
}

void RightWindow::OnURL(wxTextUrlEvent& event)
{
    //wxMessageBox(event.GetString());
	std::string sCommand(event.GetString().c_str());
	CCommandInfo::pointer commandInfo;
	if (m_commands.find(sCommand, commandInfo))
	{
		CFromInfo::pointer fromInfo = commandInfo->conversationInfo()->fromInfo();
		CUserInfo::pointer fromUserInfo = fromInfo->fromUser();
		CUserInfo::pointer sayUserInfo = commandInfo->conversationInfo()->sayAccount();
		CMessageInfo::pointer messageInfo = commandInfo->conversationInfo()->message();

		switch (commandInfo->command())
		{
		case CCommandInfo::CT_Accept:
			{
				// ??? continue;
				//m_biwoo.msgRejectFile(fromUserInfo, messageInfo->messageid());

			}break;
		case CCommandInfo::CT_SaveAs:
			{
				wxString filename = messageInfo->tostring();
				wxString title = gLangText.textSaveFileto();
				//title.append(m_curUserInfo->getUserName());
				wxString wildcards = gLangText.textAllFiles();
				wildcards.append("(*.*)|*.*");
				wxFileDialog dialog(this, title,
					wxEmptyString, filename, wildcards,
					wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

				if (dialog.ShowModal() == wxID_OK)
				{
					m_commands.remove(sCommand);
					wxRichTextCtrl& r = *m_richHistory;
					//r.Remove(commandInfo->commandPos().from, commandInfo->commandPos().to);
					//r.Replace(commandInfo->commandPos().from, commandInfo->commandPos().to, "File Transmitting");

					wxArrayString paths, filenames;
					dialog.GetPaths(paths);
					dialog.GetFilenames(filenames);

					CMessagePos::pointer messagePos;
					if (m_msgpos.find(messageInfo->messageid(), messagePos))
					{
						messagePos->filepath(paths[0].c_str());
						messagePos->filename(filenames[0].c_str());
						wxString filename = filenames[0];
						wxString sFilesize = getFilesize(messageInfo->filesize());

						filename.append(sFilesize);
						while (filename.size() < 80)
							filename.append(" ");

						//int offset = filenames[0].size() - (messagePos->filenamePos().to - messagePos->filenamePos().from);

						r.SetSelection(messagePos->filenamePos().from, messagePos->filenamePos().to);
						r.BeginStyle(m_noneUrlStyle);
						r.BeginURL(wxT(""));
						r.Replace(messagePos->filenamePos().from, messagePos->filenamePos().to, filename.c_str());
						r.EndURL();
						r.EndStyle();

						//messagePos->offset(offset);
					}

					m_biwoo.msgAcceptFile(fromUserInfo, messageInfo->messageid(), paths[0].c_str(), filenames[0].c_str());
				}
			}break;
		case CCommandInfo::CT_Reject:
			{
				m_commands.remove(sCommand);
				m_biwoo.msgRejectFile(fromUserInfo, messageInfo->messageid());
			}break;
		case CCommandInfo::CT_OpenFile:
			{
				CMessagePos::pointer messagePos;
				if (m_recvpos.find(messageInfo->messageid(), messagePos))
				{
					FileExec(messagePos->filepath());
				}
			}break;
		case CCommandInfo::CT_OpenFolder:
			{
				CMessagePos::pointer messagePos;
				if (m_recvpos.find(messageInfo->messageid(), messagePos))
				{
#ifdef WIN32
					wxString sParameter = wxString::Format("/select,%s", messagePos->filepath().c_str());
					ShellExecute((HWND)this->GetHandle(), NULL, "explorer", sParameter.c_str(), NULL, SW_SHOWNORMAL);
#else
					// ??
					FileExec(messagePos->filepath());
#endif
				}
			}break;
		case CCommandInfo::CT_Cancel:
			{
				m_commands.remove(sCommand);
				m_biwoo.msgCancelFile(fromUserInfo, messageInfo->messageid());
			}break;
		default:
			break;
		}
	}
}


void RightWindow::OnSize(wxSizeEvent& event)
{
	m_splitter->SetSize(event.GetSize());
}

void RightWindow::reset(void)
{
	m_curFromInfo.reset();
	clearMessage();

	m_staName->SetLabel("");
	m_staUserEMail->SetLabel("");
	m_staUserPhone->SetLabel("");
}

//void RightWindow::showHistory(CAccountConversation::pointer account)
//{
//
//}

void RightWindow::setCurrentSelect(CFromInfo::pointer fromInfo)
{
	assert (fromInfo.get() != NULL);

	if (isCurrentSelect(fromInfo))
	{
		return;
	}

	reset();
	m_curFromInfo = fromInfo;

	switch (fromInfo->fromType())
	{
	case CFromInfo::FromCompany:
		{
			CCompanyInfo::pointer curCompany = fromInfo->fromCompany();
			m_btnSendFile->Enable(false);
			m_btnVideoCall->Enable(false);
			m_btnSendMsg->Enable(false);
			m_btnAddUser->Enable(false);

			m_staName->SetLabel(curCompany->name());
			m_staUserEMail->SetLabel(gLangText.textCompanyType());
		}break;
	case CFromInfo::FromCoGroup:
		{
			CCoGroupInfo::pointer curCoGroupInfo = fromInfo->fromCoGroup();
			m_btnSendFile->Enable(false);
			m_btnVideoCall->Enable(false);
			m_btnSendMsg->Enable(false);
			m_btnAddUser->Enable(false);

			m_staName->SetLabel(curCoGroupInfo->name());
			if (curCoGroupInfo->type() == CCoGroupInfo::GT_Group)
			{
				m_staUserEMail->SetLabel(gLangText.textGroupType());
			}else if (curCoGroupInfo->type() == CCoGroupInfo::GT_Normal)
			{
				m_staUserEMail->SetLabel(gLangText.textCoGroupType());
			}
			wxString temp = getParentCoGroup(fromInfo);
			m_staUserDepartment->SetLabel(temp);
		}break;
	case CFromInfo::FromUserInfo:
		{
			CUserInfo::pointer curUserInfo = m_curFromInfo->fromUser();
			m_btnSendFile->Enable(true);
			m_btnVideoCall->Enable(true);
			m_btnSendMsg->Enable(true);
			m_btnAddUser->Enable(true);

			if (m_splitter->GetWindow1() != m_panelUserInfo)
			{
				m_panelUserInfo->Show(true);
				m_panelVideo->Show(false);
				m_panelGroup->Show(false);
				m_splitter->ReplaceWindow(m_splitter->GetWindow1(), m_panelUserInfo);
			}

			wxString temp;
			if (curUserInfo->getLineState() == CUserInfo::UserOnLineState)
				temp = wxString::Format(_T("%s (%s)"), curUserInfo->getUserName().c_str(), gLangText.textOnlineState().c_str());
			else if (curUserInfo->getLineState() == CUserInfo::UserAwayState)
				temp = wxString::Format(_T("%s (%s)"), curUserInfo->getUserName().c_str(), gLangText.textAwayState().c_str());
			else
				temp = wxString::Format(_T("%s (%s)"), curUserInfo->getUserName().c_str(), gLangText.textOfflineState().c_str());
			m_staName->SetLabel(temp);

			m_staUserEMail->SetLabel(curUserInfo->getEmail());
			m_staUserPhone->SetLabel(curUserInfo->getPhone());
			temp = getParentCoGroup(fromInfo);
			m_staUserDepartment->SetLabel(temp);

			static bool gFit = false;
			if (!gFit)
			{
				gFit = true;
				m_sizerUserinfo->Fit(m_panelUserInfo);
			}
		}break;
	case CFromInfo::FromDialogInfo:
		{
			m_panelGroup->cleanUser();
			m_btnSendFile->Enable(false);
			m_btnVideoCall->Enable(false);
			m_btnSendMsg->Enable(true);
			m_btnAddUser->Enable(true);
		}break;
	default:
		break;
	}

	CAccountConversation::pointer accountConversation = m_biwoo.getAccountConversations(m_curFromInfo);
	if (accountConversation.get() == NULL)
	{
		return;
	}

	//clearMessage();

	boost::mutex::scoped_lock lock(accountConversation->m_conversations.mutex());
	CLockList<CConversationInfo::pointer>::iterator iter;
	for (iter=accountConversation->m_conversations.begin(); iter!=accountConversation->m_conversations.end(); iter++)
	{
		addMessage(*iter);
	}

}

wxString RightWindow::getParentCoGroup(CFromInfo::pointer fromInfo) const
{
	assert (fromInfo.get() != NULL);

	wxString result;

	switch (fromInfo->fromType())
	{
	case CFromInfo::FromUserInfo:
		{
			CCoGroupInfo::pointer cogroupInfo = m_biwoo.getUserInfo(fromInfo->fromUser());
			assert (cogroupInfo.get() != NULL);

			result = cogroupInfo->name();
			wxString temp = getParentCoGroup(CFromInfo::create(cogroupInfo));
			result.insert(0, " > ");
			result.insert(0, temp);
		}break;
	case CFromInfo::FromCoGroup:
		{
			CCoGroupInfo::pointer cogroupInfo = fromInfo->fromCoGroup();
			CCoGroupInfo::pointer parentCoGroup = cogroupInfo->parentgroup();
			while (parentCoGroup.get() != NULL)
			{
				if (result.empty())
				{
					result = parentCoGroup->name();
				}else
				{
					result.insert(0, wxT(" > "));
					result.insert(0, parentCoGroup->name());
				}
				parentCoGroup = parentCoGroup->parentgroup();
			}

			if (result.empty())
			{
				result = cogroupInfo->companyInfo()->name();
			}else
			{
				result.insert(0, wxT(" > "));
				result.insert(0, cogroupInfo->companyInfo()->name());
			}
		}break;
	default:
		break;
	}

	return result;
}

bool RightWindow::isCurrentSelect(CFromInfo::pointer fromInfo) const
{
	assert (fromInfo.get() != NULL);
	if (m_curFromInfo.get() == NULL) return false;

	if (fromInfo->fromType() == CFromInfo::FromUserInfo && m_curFromInfo->fromType() == CFromInfo::FromUserInfo)
	{
		return fromInfo->fromUser()->getAccount() == m_curFromInfo->fromUser()->getAccount();
	}else if (fromInfo->fromType() == CFromInfo::FromDialogInfo && m_curFromInfo->fromType() == CFromInfo::FromDialogInfo)
	{
		return fromInfo->fromDialog()->dialogId() == m_curFromInfo->fromDialog()->dialogId();
	}
	return false;
}

wxString RightWindow::getFilesize(unsigned long filesize) const
{
	wxString sFilesize(" (");
	if (filesize >= 1024*1024*1024)
	{
		// G
		sFilesize.append(wxString::Format("%.02f GB", filesize / (1024*1024*1024.0)));
	}else if (filesize >= 1024*1024)
	{
		// M
		sFilesize.append(wxString::Format("%.02f MB", filesize / (1024*1024.0)));
	}else if (filesize >= 1024)
	{
		// K
		sFilesize.append(wxString::Format("%.02f KB", filesize / 1024.0));
	}else
	{
		// B
		sFilesize.append(wxString::Format("%d B", filesize));
	}
	sFilesize.append(")");
	return sFilesize;
}

void RightWindow::invitedUser(CUserInfo::pointer inviteUserInfo)
{
	assert (m_curFromInfo->fromType() == CFromInfo::FromDialogInfo);

	if (m_splitter->GetWindow1() != m_panelGroup)
	{
		m_panelUserInfo->Show(false);
		m_panelVideo->Show(false);
		m_panelGroup->Show(true);
		m_splitter->ReplaceWindow(m_splitter->GetWindow1(), m_panelGroup);
	}

	m_panelGroup->invitedUser(inviteUserInfo);
}

void RightWindow::clearMessage(void)
{
    wxRichTextCtrl& r = *m_richHistory;

	if (!m_recvpos.empty())
	{
		r.MoveEnd();
		r.WriteText(wxT(" "));
	}
	r.Clear();

	m_currentShowDay = 0;
	m_commands.clear();
	m_msgpos.clear();
	m_recvpos.clear();
}

void RightWindow::addMsgDate(time_t msgtime)
{
    wxRichTextCtrl& r = *m_richHistory;

	struct tm  * tmVal = gmtime(&msgtime);

	int msgDay = (1900+tmVal->tm_year)*10000 + (1+tmVal->tm_mon)*100 + tmVal->tm_mday;
	if (m_currentShowDay != msgDay)
	{
		m_currentShowDay = msgDay;

		char dateBuffer[40];
		sprintf(dateBuffer, "%04d-%02d-%02d", 1900+tmVal->tm_year, 1+tmVal->tm_mon, tmVal->tm_mday);

		r.BeginLeftIndent(20);
		r.BeginStyle(m_dateStyle);
		r.WriteText(dateBuffer);
		r.EndStyle();
		r.Newline();
		r.EndLeftIndent();
	}
}

void RightWindow::addMessage(CConversationInfo::pointer conversationInfo)
{
	assert (conversationInfo.get() != NULL);

    wxRichTextCtrl& r = *m_richHistory;

	if (conversationInfo->message()->type() == 11)
	{
		CMessagePos::pointer messagePos;
		if (m_msgpos.find(conversationInfo->message()->messageid(), messagePos, false))
		{
			CCommandInfo::pointer command1 = messagePos->cmd1();
			CCommandInfo::pointer command2 = messagePos->cmd2();

			//static bool bRemove = false;
			switch (conversationInfo->conversationType())
			{
			case CConversationInfo::CT_ACCEPT:
				{
					return;
				}break;
			case CConversationInfo::CT_CANCEL:
			case CConversationInfo::CT_REJECT:
				{
					wxString sTemp = conversationInfo->conversationType() == CConversationInfo::CT_CANCEL ? gLangText.textFileCanceled() : gLangText.textFileRejected();
					while (sTemp.size() < command2->commandPos().to - command1->commandPos().from)
						sTemp.append(" ");

					r.BeginStyle(m_noneUrlStyle);
					r.BeginURL(wxT(""));
					r.SetSelection(command1->commandPos().from, command2->commandPos().to);
					r.Replace(command1->commandPos().from, command2->commandPos().to, sTemp);
					r.EndURL();
					r.EndStyle();

					m_commands.remove(command1->url());
					m_commands.remove(command2->url());

					m_msgpos.remove(conversationInfo->message()->messageid());
					return;
				}
			default:
				break;
			}

			float percent = conversationInfo->percent();
			bool finished = percent == 100.0;
			if (finished)
			{
				//bRemove = false;
				bool bOwnerMsg = conversationInfo->sayAccount().get() == gMyAccount->getUserinfo().get();

				if (!bOwnerMsg)
				{
					messagePos->filepath(conversationInfo->message()->tostring());

					m_recvpos.insert(conversationInfo->message()->messageid(), messagePos);
				}

				if (bOwnerMsg || (r.GetLastPosition() > messagePos->messagePos().to+1))
				{
					wxString sTemp = bOwnerMsg ? gLangText.textFileSent() : gLangText.textFileArrived();
					while (sTemp.size() < command2->commandPos().to - command1->commandPos().from)
						sTemp.append(" ");

					r.BeginStyle(m_noneUrlStyle);
					r.BeginURL(wxT(""));
					r.SetSelection(command1->commandPos().from, command2->commandPos().to);
					r.Replace(command1->commandPos().from, command2->commandPos().to, sTemp);
					r.EndURL();
					r.EndStyle();

					m_commands.remove(command1->url());
					m_commands.remove(command2->url());
					if (bOwnerMsg)
					{
						return;
					}
				}else
				{
					wxString sCmdOpenFile = gLangText.btnOpenFile();
					wxString sCmdOpenFolder = gLangText.btnOpenFolder();

					r.SetSelection(command1->commandPos().from, command1->commandPos().from+sCmdOpenFile.size());
					r.BeginStyle(m_urlStyle);
					r.BeginURL(command1->url());
					r.Replace(command1->commandPos().from, command1->commandPos().from+sCmdOpenFile.size(), sCmdOpenFile);
					r.EndURL();
					r.EndStyle();

					r.SetSelection(command2->commandPos().from, command2->commandPos().from+sCmdOpenFolder.size());
					r.BeginStyle(m_urlStyle);
					r.BeginURL(command2->url());
					r.Replace(command2->commandPos().from, command2->commandPos().from+sCmdOpenFolder.size(), sCmdOpenFolder);
					r.EndURL();
					r.EndStyle();

					command1->command(CCommandInfo::CT_OpenFile);
					command2->command(CCommandInfo::CT_OpenFolder);

					m_commands.insert(command1->url(), command1);
					m_commands.insert(command2->url(), command2);
					return;
				}
			}else
			{
				wxString sTemp = wxString::Format("%.02f%%", percent);

				//if (!bRemove)
				if (percent == 0.0)
				{
					//bRemove = true;
					wxString sSpaceTemp(" ");
					while (sSpaceTemp.size() < command1->commandPos().to - command1->commandPos().from)
						sSpaceTemp.append(" ");
					r.BeginStyle(m_noneUrlStyle);
					r.BeginURL(wxT(""));
					r.SetSelection(command1->commandPos().from, command1->commandPos().to);
					r.Replace(command1->commandPos().from, command1->commandPos().to, sSpaceTemp);
					r.EndURL();
					r.EndStyle();
				}

				r.BeginStyle(m_noneUrlStyle);
				r.BeginURL(wxT(""));
				r.SetSelection(command1->commandPos().from, command1->commandPos().from+sTemp.size());
				r.Replace(command1->commandPos().from, command1->commandPos().from+sTemp.size(), sTemp);
				r.EndURL();
				r.EndStyle();

				return;
			}
		}
	}else if (conversationInfo->message()->type() == 13)
	{
		switch (conversationInfo->conversationType())
		{
		case CConversationInfo::CT_MSG:
			{
				if (!m_showVideoPanel)
				{
					m_showVideoPanel = true;
					m_panelUserInfo->Show(!m_showVideoPanel);
					m_panelVideo->Show(m_showVideoPanel);
					m_splitter->ReplaceWindow(m_panelUserInfo, m_panelVideo);
				}

				CFromInfo::pointer fromInfo = conversationInfo->fromInfo();
				CUserInfo::pointer userInfo = fromInfo->fromUser();
				assert (userInfo.get() != NULL);
				bool bOwnerMsg = conversationInfo->sayAccount().get() == gMyAccount->getUserinfo().get();
				m_panelVideo->SetVideoCall(bOwnerMsg, userInfo, conversationInfo->message()->messageid());
			}break;
		case CConversationInfo::CT_CANCEL:
			{
				if (m_showVideoPanel)
				{
					m_showVideoPanel = false;
					m_panelUserInfo->Show(!m_showVideoPanel);
					m_panelVideo->Show(m_showVideoPanel);
					m_splitter->ReplaceWindow(m_panelVideo, m_panelUserInfo);
				}

				m_richTextCtrl->SetFocus();
			}break;
		default:
			break;
		}

		//HWND hWndPreview = m_panelVideo->GetRemoteHwnd();
		//m_biwoo.acceptVideoCall(userInfo, hWndPreview, conversationInfo->message()->messageid());
		return;
	}else if (conversationInfo->message()->type() == 601)
	{
		time_t msgtime = conversationInfo->message()->msgtime();
		addMsgDate(msgtime);

		r.MoveEnd();
		r.BeginLeftIndent(60);

		wxString sMessage;
		if (conversationInfo->conversationType() == CConversationInfo::CT_CREATE)
		{
			r.BeginBold();
			r.WriteText(conversationInfo->sayAccount()->getUserName());
			r.EndBold();
			r.WriteText(wxString::Format(" %s", "created GroupDialog"));
		}else if (conversationInfo->conversationType() == CConversationInfo::CT_INVITE)
		{
			r.BeginBold();
			r.WriteText(conversationInfo->fromAccount()->getUserName());
			r.EndBold();
			r.WriteText(wxString::Format(" %s ", "added"));
			r.BeginBold();
			r.WriteText(conversationInfo->sayAccount()->getUserName());
			r.EndBold();
		}

		r.Newline();
		r.EndLeftIndent();
		r.ShowPosition(r.GetLastPosition());
		conversationInfo->readState(true);
		return;
	}

	bool bOwnerMsg = conversationInfo->sayAccount().get() == gMyAccount->getUserinfo().get();

    wxString lineBreak = (wxChar) 29;
    r.BeginSuppressUndo();
	r.MoveEnd();
	if (conversationInfo->message()->newflag())
	{
		time_t msgtime = conversationInfo->message()->msgtime();
		addMsgDate(msgtime);
		struct tm  * tmVal = gmtime(&msgtime);

		char timeBuffer[40];
		sprintf(timeBuffer, "%02d:%02d:%02d", tmVal->tm_hour, tmVal->tm_min, tmVal->tm_sec);

		wxString sSayAccount(conversationInfo->sayAccount()->getUserName());
		sSayAccount.append(" ");
		sSayAccount.append(timeBuffer);
		r.BeginLeftIndent(0);
		if (bOwnerMsg)
			r.BeginStyle(m_ownerStyle);
		else
			r.BeginStyle(m_blueStyle);
		r.WriteText(sSayAccount);
		r.EndStyle();
		r.Newline();
		r.EndLeftIndent();
	}else
	{
		r.MoveLeft();
	}

	r.BeginLeftIndent(60);
	switch (conversationInfo->message()->type())
	{
	case 1:
		{
			wxString sMessage(conversationInfo->message()->tostring());
			r.WriteText(sMessage);
		}break;
	case 3:
		{
			size_t imageSize = 0;
			wxImage image(conversationInfo->message()->imageWidth(), conversationInfo->message()->imageHeight(), (unsigned char *)conversationInfo->message()->getdata(), true);
			if (image.IsOk())
			{
				r.WriteImage(image, wxBITMAP_TYPE_BMP);
			}
		}break;
	case 11:
		{
			float percent = conversationInfo->percent();
			bool finished = percent == 100.0;

			CMessagePos::pointer messagePosOld;
			if (finished)
			{
				if (m_msgpos.find(conversationInfo->message()->messageid(), messagePosOld, true))
				{
					//CCommandInfo::pointer command1 = messagePosOld->cmd1();
					//CCommandInfo::pointer command2 = messagePosOld->cmd2();
				}
			}

			if (finished && messagePosOld.get() == NULL)
			{
				// error
				break;
			}

			wxString sCommandUrl1 = wxString::Format("1_%d", conversationInfo->message()->messageid());
			wxString sCommandUrl2 = wxString::Format("2_%d", conversationInfo->message()->messageid());
			wxString sFilename = messagePosOld.get() == NULL ? conversationInfo->message()->tostring() : messagePosOld->filename();
			
			if (!bOwnerMsg)
			//if (conversationInfo->conversationType() == CConversationInfo::CT_SUCCEED && !bOwnerMsg)
			{
				sFilename = sFilename.AfterLast(_T('\\'));
			}

			if (messagePosOld.get() == NULL)
			{
				// Add file icon.
				bool writeFileTypeImage = false;
				wxString ext = sFilename.AfterLast(_T('.'));
				wxFileType * ft = wxTheMimeTypesManager->GetFileTypeFromExtension(ext);
				if (ft != NULL)
				{
					wxIconLocation iconLocation;
					if (ft->GetIcon(&iconLocation))
					{
						wxBitmap bitmap;
						if (bitmap.CopyFromIcon(wxIcon(iconLocation)))
						{
							r.WriteImage(bitmap, wxBITMAP_TYPE_BMP);
							writeFileTypeImage = true;
						}
					}
					delete ft;
				}
				if (!writeFileTypeImage)
				{
					const wxSize imageSize(32, 32);
					wxBitmap bitmap = wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, imageSize);
					if (bitmap.IsOk())
					{
						r.WriteImage(bitmap, wxBITMAP_TYPE_BMP);
					}
				}
			}

			// add file size
			wxString sFilesize = getFilesize(conversationInfo->message()->filesize());
			sFilename.append(sFilesize);
			while (sFilename.size() < 80)
				sFilename.append(" ");

			RTPOS posFilename;
			posFilename.from = r.GetLastPosition();
			r.WriteText(sFilename);
			//wxRichTextImageBlock rtImageBlock()
			//r.WriteImage(rtImageBlock);
			posFilename.to = r.GetLastPosition();
			r.Newline();
			if (conversationInfo->conversationType() == CConversationInfo::CT_HISTORY)
			{
				r.WriteText(gLangText.textWaitForReceive());
				break;
			}else if (conversationInfo->conversationType() == CConversationInfo::CT_CANCEL)
			{
				r.WriteText(gLangText.textFileCanceled());
				break;
			}else if (conversationInfo->conversationType() == CConversationInfo::CT_REJECT)
			{
				r.WriteText(gLangText.textFileRejected());
				break;
			}else if (conversationInfo->conversationType() == CConversationInfo::CT_ACCEPT)
			{
				r.WriteText(gLangText.textAcceptFile());
				break;
			}else if (conversationInfo->conversationType() == CConversationInfo::CT_SUCCEED)
			{
				if (bOwnerMsg)
				{
					r.WriteText(gLangText.textFileSent());
					break;
				}else
				{
					//r.WriteText(bOwnerMsg ? gLangText.textFileSent() : gLangText.textFileArrived());
				}
				//break;
			}


			RTPOS posMessage;
			posMessage.from = r.GetLastPosition();

			wxString sCommand1;
			wxString sCommand2;
			CCommandInfo::CommandType commandType1 = CCommandInfo::CT_None;
			CCommandInfo::CommandType commandType2 = CCommandInfo::CT_None;
			if (bOwnerMsg)
			{
				sCommand1 = gLangText.textWaitForReceive();
				sCommand2 = gLangText.btnCancelFile();
				commandType2 = CCommandInfo::CT_Cancel;
			}else if (conversationInfo->conversationType() == CConversationInfo::CT_SUCCEED)
			{
				sCommand1 = gLangText.btnOpenFile();
				sCommand2 = gLangText.btnOpenFolder();
				commandType1 = CCommandInfo::CT_OpenFile;
				commandType2 = CCommandInfo::CT_OpenFolder;
			}else
			{
				sCommand1 = messagePosOld.get() == NULL ? gLangText.btnSaveFileAs() : gLangText.btnOpenFile();
				sCommand2 = messagePosOld.get() == NULL ? gLangText.btnRejectFile() : gLangText.btnOpenFolder();
				commandType1 = messagePosOld.get() == NULL ? CCommandInfo::CT_SaveAs : CCommandInfo::CT_OpenFile;
				commandType2 = messagePosOld.get() == NULL ? CCommandInfo::CT_Reject : CCommandInfo::CT_OpenFolder;
			}

			// SAVE AS / OPEN FILE
			wxString sSpaceTemp(wxT("  "));
			while ((sSpaceTemp.size() + sCommand1.size()) < 20)
				sSpaceTemp.append("  ");

			RTPOS pos1;
			pos1.from = r.GetLastPosition();
			if (!bOwnerMsg)
			{
				r.BeginStyle(m_urlStyle);
				r.BeginURL(sCommandUrl1);
			}
			r.WriteText(sCommand1);
			if (!bOwnerMsg)
			{
				r.EndURL();
				r.EndStyle();
			}
			pos1.to = r.GetLastPosition();
			r.WriteText(sSpaceTemp);


			// REJECT / OPEN DIR
			sSpaceTemp = (wxT("  "));
			while ((sSpaceTemp.size() + sCommand2.size()) < 20)
				sSpaceTemp.append("  ");

			RTPOS pos2;
			pos2.from = r.GetLastPosition();
			r.BeginStyle(m_urlStyle);
			r.BeginURL(sCommandUrl2);
			r.WriteText(sCommand2);
			r.EndURL();
			r.EndStyle();
			pos2.to = r.GetLastPosition();
			r.WriteText(sSpaceTemp);
			posMessage.to = r.GetLastPosition();

			CCommandInfo::pointer commandInfo1 = CCommandInfo::create(sCommandUrl1.c_str(), commandType1, conversationInfo, pos1);
			if (!bOwnerMsg)
			{
				m_commands.insert(sCommandUrl1.c_str(), commandInfo1);
			}

			CCommandInfo::pointer commandInfo2 = CCommandInfo::create(sCommandUrl2.c_str(), commandType2, conversationInfo, pos2);
			m_commands.insert(sCommandUrl2.c_str(), commandInfo2);

			if (messagePosOld.get() == NULL)
			{
				CMessagePos::pointer messagePos = CMessagePos::create(conversationInfo->message()->messageid(), posMessage, posFilename);
				messagePos->cmd1(commandInfo1);
				messagePos->cmd2(commandInfo2);
				m_msgpos.insert(conversationInfo->message()->messageid(), messagePos);

				if (conversationInfo->conversationType() == CConversationInfo::CT_SUCCEED)
				{
					messagePos->filepath(conversationInfo->message()->tostring());
					m_recvpos.insert(conversationInfo->message()->messageid(), messagePos);
				}
			}
		}break;
	default:
		break;
	}
	if (conversationInfo->message()->newflag())
	{
		r.Newline();
	}
	r.EndLeftIndent();
    r.EndSuppressUndo();
	r.ShowPosition(r.GetLastPosition());
	conversationInfo->readState(true);
}

void RightWindow::OnSendMsg(wxCommandEvent& event)
{
	assert (gMyAccount.get() != NULL);

	if (m_curFromInfo.get() == NULL)
	{
		wxMessageBox(gLangText.textSelectUserTip(), _T("biwoo"), wxOK | wxICON_WARNING, this);
		m_richTextCtrl->SetFocus();
		return;
	}
	if (!m_richTextCtrl->IsModified())
	{
		m_richTextCtrl->SetFocus();
		return;
	}

	wxRichTextBuffer & buffer = m_richTextCtrl->GetBuffer();
    wxTextAttrEx currentParaStyle = buffer.GetAttributes();
    wxTextAttrEx currentCharStyle = buffer.GetAttributes();

	bool newState = true;

	wxString sTextToSend = "";
	struct timeb tbNow;
	ftime(&tbNow);
	tbNow.time -= (tbNow.timezone*60);

    wxRichTextObjectList::compatibility_iterator node = buffer.GetChildren().GetFirst();
    while (node)
    {
        wxRichTextParagraph* para = wxDynamicCast(node->GetData(), wxRichTextParagraph);
        wxASSERT (para != NULL);

        if (para)
        {
            wxTextAttrEx paraStyle(para->GetCombinedAttributes());

            //BeginParagraphFormatting(currentParaStyle, paraStyle, str);

            wxRichTextObjectList::compatibility_iterator node2 = para->GetChildren().GetFirst();
            while (node2)
            {
                wxRichTextObject* obj = node2->GetData();
                wxRichTextPlainText* textObj = wxDynamicCast(obj, wxRichTextPlainText);
                if (textObj && !textObj->IsEmpty())
                {
                    wxTextAttrEx charStyle(para->GetCombinedAttributes(obj->GetAttributes()));
                    //BeginCharacterFormatting(currentCharStyle, charStyle, paraStyle, str);

                    const wxString & text = textObj->GetText();
					if (!sTextToSend.IsEmpty())
					{
						sTextToSend.append("\n");
					}
					sTextToSend.append(text);

                    //if (charStyle.HasTextEffects() && (charStyle.GetTextEffects() & wxTEXT_ATTR_EFFECT_CAPITALS))
                    //    text.MakeUpper();

                    //wxString toReplace = wxRichTextLineBreakChar;
                    //text.Replace(toReplace, wxT("<br>"));

                    //str << text;

                    //EndCharacterFormatting(currentCharStyle, charStyle, paraStyle, str);
                }

                wxRichTextImage* image = wxDynamicCast(obj, wxRichTextImage);
                if( image && (!image->IsEmpty() || image->GetImageBlock().GetData()))
				{
					if (!sTextToSend.IsEmpty())
					{
						CMessageInfo::pointer messageInfo = CMessageInfo::create(0, sTextToSend.Length(), 1, newState);
						messageInfo->setdata(sTextToSend.c_str(), sTextToSend.Length(), 0);

						messageInfo->msgtime(tbNow.time);
						addMessage(CConversationInfo::create(m_curFromInfo, gMyAccount->getUserinfo(), messageInfo));

						m_biwoo.msgSendText(m_curFromInfo, sTextToSend.c_str(), sTextToSend.Length(), newState);
						newState = false;
						sTextToSend.clear();
					}

					//if (image->GetImageBlock().GetData() != NULL)
						//m_biwoo.msgSendImage(m_curUserInfo, (const char*)image->GetImageBlock().GetData(), image->GetImageBlock().GetDataSize(), 3, newState);
					if (!image->IsEmpty())
					{
						int width = image->GetImage().GetWidth();
						int height = image->GetImage().GetHeight();

						CMessageInfo::pointer messageInfo = CMessageInfo::create(0, width*height*3, 3, newState);
						messageInfo->setdata((const char*)image->GetImage().GetData(), width*height*3, 0);
						messageInfo->imageWH(width, height);
						messageInfo->msgtime(tbNow.time);
						addMessage(CConversationInfo::create(m_curFromInfo, gMyAccount->getUserinfo(), messageInfo));

						//int width = image->GetImage().GetWidth() + (image->GetImage().GetWidth() % 2);
						//int height = image->GetImage().GetHeight() + (image->GetImage().GetHeight() % 2);
						m_biwoo.msgSendImage(m_curFromInfo, width, height, 0, (const char*)image->GetImage().GetData(), width*height*3, newState);
						newState = false;
					}

					// image->GetImageBlock().GetDataSize();
                    //WriteImage( image, stream );
				}

                node2 = node2->GetNext();
            }

            //EndParagraphFormatting(currentParaStyle, paraStyle, str);

            //str << wxT("\n");
        }
        node = node->GetNext();
    }

	if (!sTextToSend.IsEmpty())
	{
		CMessageInfo::pointer messageInfo = CMessageInfo::create(0, sTextToSend.Length(), 1, newState);
		messageInfo->setdata(sTextToSend.c_str(), sTextToSend.Length(), 0);

		struct timeb tbNow;
		ftime(&tbNow);
		tbNow.time -= (tbNow.timezone*60);
		messageInfo->msgtime(tbNow.time);
		addMessage(CConversationInfo::create(m_curFromInfo, gMyAccount->getUserinfo(), messageInfo));

		m_biwoo.msgSendText(m_curFromInfo, sTextToSend.c_str(), sTextToSend.Length(), newState);
	}

	//wxString sText = m_richTextCtrl->GetValue();
	//addMessage(CConversationInfo::create(m_curUserInfo, gMyAccount->getUserinfo(), sText.c_str()));
	//m_biwoo.msgSend(m_curUserInfo, sText.c_str(), sText.Length());
	m_richTextCtrl->Clear();
	m_richTextCtrl->SetFocus();
}

void RightWindow::OnSendFile(wxCommandEvent& event)
{
	assert (gMyAccount.get() != NULL);

	if (m_curFromInfo.get() == 0 || m_curFromInfo->fromType() != CFromInfo::FromUserInfo)
	{
		wxMessageBox(gLangText.textSelectUserTip(), _T("biwoo"), wxOK | wxICON_WARNING, this);
		m_richTextCtrl->SetFocus();
		return;
	}

	if (m_curFromInfo->fromUser()->isOfflineState())
	{
		wxMessageBox(gLangText.textOfflineSendfileTip(), _T("biwoo"), wxOK | wxICON_WARNING, this);
		m_richTextCtrl->SetFocus();
		return;
	}

	wxString title = gLangText.textSendFileto();
	title.append(m_curFromInfo->fromUser()->getUserName());
    wxString wildcards = gLangText.textAllFiles();
	wildcards.append("(*.*)|*.*");
    wxFileDialog dialog(this, title,
                        wxEmptyString, wxEmptyString, wildcards,
                        //wxFD_OPEN);
                        wxFD_OPEN|wxFD_MULTIPLE|wxFD_PREVIEW);

    if (dialog.ShowModal() == wxID_OK)
    {
		if (m_curFromInfo->fromUser()->isOfflineState())
		{
			wxMessageBox(gLangText.textOfflineSendfileTip(), _T("biwoo"), wxOK | wxICON_WARNING, this);
			m_richTextCtrl->SetFocus();
			return;
		}

        wxArrayString paths, filenames;

        dialog.GetPaths(paths);
        dialog.GetFilenames(filenames);

        size_t count = paths.GetCount();
        for ( size_t n = 0; n < count; n++ )
        {
			if (n == 5)
			{
				//wxMessageBox(_T("Can not send file to the offline user."), _T("biwoo"), wxOK | wxICON_WARNING, this);
				m_richTextCtrl->SetFocus();
				break;
			}
			std::string filepath = paths[n].c_str();
			std::string filename = filenames[n].c_str();
			m_biwoo.msgSendFile(m_curFromInfo->fromUser(), filepath.c_str(), filename.c_str());
 
#ifdef WIN32
			Sleep(100);
#else
			usleep(100000);
#endif
        }
    }
}

void RightWindow::OnVideoCall(wxCommandEvent& event)
{
	assert (gMyAccount.get() != NULL);

	if (m_curFromInfo.get() == 0 || m_curFromInfo->fromType() != CFromInfo::FromUserInfo)
	{
		wxMessageBox(gLangText.textSelectUserTip(), _T("biwoo"), wxOK | wxICON_WARNING, this);
		m_richTextCtrl->SetFocus();
		return;
	}

	if (m_curFromInfo->fromUser()->isOfflineState())
	{
		wxMessageBox(gLangText.textOfflineVideoCallTip(), _T("biwoo"), wxOK | wxICON_WARNING, this);
		return;
	}

	m_panelVideo->videoCall(m_curFromInfo->fromUser());
}

void RightWindow::OnAddUser(wxCommandEvent& event)
{
	assert (gMyAccount.get() != NULL);

	if (m_curFromInfo.get() == NULL)
	{
		wxMessageBox(gLangText.textSelectUserTip(), _T("biwoo"), wxOK | wxICON_WARNING, this);
		m_richTextCtrl->SetFocus();
		return;
	}

	long dialogid = 0;
	CDlgAddUser dlgAddUser(this);
	if (m_curFromInfo->fromType() == CFromInfo::FromDialogInfo)
	{
		dialogid = m_curFromInfo->fromDialog()->dialogId();
		CLockMap<std::string, CUserInfoPointer>::iterator iter;
		for (iter=m_curFromInfo->fromDialog()->m_members.begin(); iter!=m_curFromInfo->fromDialog()->m_members.end(); iter++)
		{
			dlgAddUser.m_existusers.insert(iter->first, true);
		}

	}else if (m_curFromInfo->fromType() == CFromInfo::FromUserInfo)
	{
		dlgAddUser.m_existusers.insert(m_curFromInfo->fromUser()->getAccount(), true);
	}
	dlgAddUser.loadContacts();

	int ret = dlgAddUser.ShowModal();
	if (ret != wxID_OK)
	{
		return;
	}

	std::list<CUserInfo::pointer> inviteUsers;
	if (m_curFromInfo->fromType() == CFromInfo::FromUserInfo)
	{
		inviteUsers.push_back(m_curFromInfo->fromUser());
	}

	CLockMap<tstring, CUserInfo::pointer>::iterator iter;
	for (iter=dlgAddUser.m_addusers.begin(); iter!=dlgAddUser.m_addusers.end(); iter++)
	{
		CUserInfoPointer userInfo = iter->second;
		inviteUsers.push_back(userInfo);
	}

	m_biwoo.diaInvite(dialogid, inviteUsers);
}

void RightWindow::OnQuitDialog(wxCommandEvent& event)
{
	//if (m_curDialogInfo.get() == NULL) return;

	// ???
	//m_biwoo.diaQuit(m_curDialogInfo->dialogId());
}
