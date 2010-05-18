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
#include "RelationshipPanel.h"
#include "wx/taskbar.h"
#include "LeftWindow.h"
#include "RightWindow.h"
#include "RSDataPanel.h"
#include "App.h"


int EditLableType = 0;	// 1: add, 2: Edit

const int cNewMsgImage		= 2;
const int cOnlineImage		= 3;
const int cOfflineImage		= 4;

int getUserLineStateImage(CUserInfo::UserLineState userLineState)
{
	switch (userLineState)
	{
	case CUserInfo::UserOffLineState:
		return cOfflineImage;
	//case CUserInfo::UserAwayState:
		// ???
		//break;
	default:
		return cOnlineImage;
	}
}

//BEGIN_EVENT_TABLE(RelationshipPanel, wxScrolledWindow)
BEGIN_EVENT_TABLE(RelationshipPanel, wxPanel)

EVT_SIZE    (           RelationshipPanel::OnSize)
    EVT_MENU(Menu_Deletemessage, RelationshipPanel::OnMenuDeleteMessage)

    //EVT_MENU(Menu_NewRootGroup, RelationshipPanel::OnMenuNewRootGroup)
    //EVT_MENU(Menu_NewChildGroup, RelationshipPanel::OnMenuNewChildGroup)
    //EVT_MENU(Menu_DeleteGroup, RelationshipPanel::OnMenuDeleteGroup)

    //EVT_TREE_BEGIN_LABEL_EDIT(TreeTest_Ctrl, RelationshipPanel::OnBeginLabelEdit)
    //EVT_TREE_END_LABEL_EDIT(TreeTest_Ctrl, RelationshipPanel::OnEndLabelEdit)

    EVT_TREE_SEL_CHANGED(TreeTest_Ctrl, RelationshipPanel::OnSelChanged)
    //EVT_TREE_ITEM_ACTIVATED(TreeTest_Ctrl, RelationshipPanel::OnItemActivated)

#if USE_CONTEXT_MENU
    EVT_CONTEXT_MENU(RelationshipPanel::OnContextMenu)
#else
    EVT_RIGHT_UP(RelationshipPanel::OnRightUp)
#endif // USE_CONTEXT_MENU

	EVT_TIMER(wxID_ANY, RelationshipPanel::OnTimer)

END_EVENT_TABLE()

RelationshipPanel::RelationshipPanel(wxWindow * parent)
: wxPanel(parent)
, m_treeCtrl(NULL)
, m_bGetRecents(false)

{
    long style = wxTR_DEFAULT_STYLE | wxTR_EDIT_LABELS | wxSUNKEN_BORDER | wxTR_HIDE_ROOT;
//	long style = wxTR_HAS_BUTTONS|wxTR_EDIT_LABELS | wxTR_SINGLE | wxTR_EXTENDED | wxTR_HIDE_ROOT;
    m_treeCtrl = new wxTreeCtrl(this, TreeTest_Ctrl,
                                wxPoint(0,0), wxDefaultSize,
                                style);

    m_imageList = new wxImageList(16, 16, true);
    m_imageList->Add( wxIcon(gAppPath + wxT("/res/the_doghouse.ico"), wxBITMAP_TYPE_ICO) );
    m_imageList->Add( wxIcon(gAppPath + wxT("/res/flowerpot.ico"), wxBITMAP_TYPE_ICO) );
    m_imageList->Add( wxIcon(gAppPath + wxT("/res/newmsg.ico"), wxBITMAP_TYPE_ICO) );
    m_imageList->Add( wxIcon(gAppPath + wxT("/res/online.ico"), wxBITMAP_TYPE_ICO) );
    m_imageList->Add( wxIcon(gAppPath + wxT("/res/offline.ico"), wxBITMAP_TYPE_ICO) );
    //m_imageList->Add( wxIcon(gAppPath + wxT("/res/STATE_WHITE.ico"), wxBITMAP_TYPE_ICO) );
    m_treeCtrl->AssignImageList(m_imageList);
}

RelationshipPanel::~RelationshipPanel(void)
{
	m_timer.Stop();
	clearDatas();
	delete m_treeCtrl;
}

void RelationshipPanel::clearDatas(void)
{
	m_newitems.clear();
	m_cogitemdatas.clear(true, false);
	m_coitemdatas.clear(true, false);
	m_users.clear();
}

void RelationshipPanel::OnSize(wxSizeEvent& event)
{
	if (m_treeCtrl)
		m_treeCtrl->SetSize(event.GetSize());
}

void RelationshipPanel::buildAllContacts(void)
{
	BOOST_ASSERT (gMyCompany == (CbiwooHandler*)this);
	if (m_treeCtrl == NULL) return;

	if (!m_allContactsItem.IsOk())
	{
		m_allContactsItem = m_treeCtrl->InsertItem(TVI_ROOT, -1, gLangText.textAllContacts(), 1);
	}
}

void RelationshipPanel::buildConversations(void)
{
	BOOST_ASSERT (gMyCoGroup == (CbiwooHandler*)this);
	if (m_treeCtrl == NULL) return;

	if (!m_conversationsItem.IsOk())
	{
		m_conversationsItem = m_treeCtrl->InsertItem(TVI_ROOT, -1, gLangText.textConversations(), 1);

		// Create thread to load the conversations info.
		m_timer.SetOwner(this);
		m_timer.Start(200);
	}
}

void RelationshipPanel::DoSearchUsers(const wxString & searchString)
{
	BOOST_ASSERT (gMyCompany == (CbiwooHandler*)this);

	m_treeCtrl->DeleteChildren(m_allContactsItem);
	CLockMap<unsigned long, CUserInfo::pointer>::iterator iter;
	for (iter=m_users.begin(); iter!=m_users.end(); iter++)
	{
		CUserInfo::pointer userInfo = iter->second;

		if (strstr(userInfo->getAccount().c_str(), searchString.c_str()) != NULL ||
			strstr(userInfo->getUserName().c_str(), searchString.c_str()) != NULL ||
			strstr(userInfo->getNick().c_str(), searchString.c_str()) != NULL ||
			strstr(userInfo->getPhone().c_str(), searchString.c_str()) != NULL ||
			strstr(userInfo->getEmail().c_str(), searchString.c_str()) != NULL
			)
		{
			wxString sItem = wxString::Format("%s(%s)", userInfo->getUserName().c_str(), userInfo->getAccount().c_str());

			wxTreeItemId itemNew = m_treeCtrl->InsertItem(m_allContactsItem, -1, sItem, -1);
			CTreeItemData * itemDataNew = new CTreeItemData(itemNew, userInfo);
			m_treeCtrl->SetItemData(itemNew, itemDataNew);
		}
	}

	m_treeCtrl->Expand(m_allContactsItem);
}

void RelationshipPanel::OnTimer(wxTimerEvent & event)
{
	BOOST_ASSERT (theFrame != NULL);
	BOOST_ASSERT (gMyCoGroup == (CbiwooHandler*)this);
	BOOST_ASSERT (m_conversationsItem.IsOk());

	if (m_bGetRecents)
	{
		m_bGetRecents = false;
		const CLockList<CFromInfo::pointer>	& recents = m_biwoo.getRecents();
		CLockList<CFromInfo::pointer>::const_iterator iter;
		for (iter=recents.begin(); iter!=recents.end(); iter++)
		{
			CFromInfo::pointer fromInfo = *iter;
			if (fromInfo->fromType() == CFromInfo::FromUserInfo)
			{
				CTreeItemData * finditemUser = findChildItem(m_conversationsItem, fromInfo->fromUser());
				if (finditemUser == NULL)
				{
					wxTreeItemId itemNewConversation = m_treeCtrl->InsertItem(m_conversationsItem, -1, fromInfo->fromUser()->getUserName(), getUserLineStateImage(fromInfo->fromUser()->getLineState()));
					m_treeCtrl->Expand(m_conversationsItem);

					CTreeItemData * itemDataNew = new CTreeItemData(itemNewConversation, fromInfo->fromUser());
					m_treeCtrl->SetItemData(itemNewConversation, itemDataNew);
				}
			}else if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
			{
				CDialogInfo::pointer dialogInfo = fromInfo->fromDialog();
				CTreeItemData * finditemUser = findChildItem(m_conversationsItem, dialogInfo);
				if (finditemUser == NULL)
				{
					wxString sDialogAccountString = getDialogInfoAccount(dialogInfo);

					wxTreeItemId itemNewConversation = m_treeCtrl->InsertItem(m_conversationsItem, -1, sDialogAccountString, cOnlineImage);
					m_treeCtrl->Expand(m_conversationsItem);

					CTreeItemData * itemDataNew = new CTreeItemData(itemNewConversation, dialogInfo);
					m_treeCtrl->SetItemData(itemNewConversation, itemDataNew);
				}
			}
		}
		
	}

	CConversationInfo::pointer conversationInfo = m_biwoo.getUnreadConversation();
	if (conversationInfo.get() != NULL)
	{
		CFromInfo::pointer fromInfo = conversationInfo->fromInfo();
		CDialogInfo::pointer fromDialogInfo = fromInfo->fromDialog();
		CUserInfo::pointer fromUserInfo = fromInfo->fromUser();
		CUserInfo::pointer sayUserInfo = conversationInfo->sayAccount();
		BOOST_ASSERT (sayUserInfo.get() != NULL);

		if (sayUserInfo.get() == gMyAccount->getUserinfo().get())
		{
			if ((fromDialogInfo.get() != NULL && gRightWindow->isCurrentSelect(fromInfo)) ||
				(fromUserInfo.get() != NULL && gRightWindow->isCurrentSelect(fromInfo)))
			{
				gRightWindow->addMessage(conversationInfo);
			}
			return;
		}
		wxTreeItemId itemNewConversation;
		//CTreeItemData * finditemUser = findChildItem(m_conversationsItem, sayUserInfo);
		CTreeItemData * finditemUser = NULL;
		if (fromInfo->fromType() == CFromInfo::FromUserInfo)
		{
			finditemUser = findChildItem(m_conversationsItem, fromInfo->fromUser());
			if (finditemUser == NULL)
			{
				itemNewConversation = m_treeCtrl->InsertItem(m_conversationsItem, -1, sayUserInfo->getUserName(), getUserLineStateImage(sayUserInfo->getLineState()));
				m_treeCtrl->Expand(m_conversationsItem);

				CTreeItemData * itemDataNew = new CTreeItemData(itemNewConversation, sayUserInfo);
				m_treeCtrl->SetItemData(itemNewConversation, itemDataNew);
			}else
			{
				itemNewConversation = finditemUser->GetId();
			}

		}else if (fromInfo->fromType() == CFromInfo::FromDialogInfo)
		{
			CDialogInfo::pointer dialogInfo = fromInfo->fromDialog();
			wxString sDialogAccountString = getDialogInfoAccount(dialogInfo);
			finditemUser = findChildItem(m_conversationsItem, dialogInfo);

			if (finditemUser == NULL)
			{
				itemNewConversation = m_treeCtrl->InsertItem(m_conversationsItem, -1, sDialogAccountString, cOnlineImage);
				m_treeCtrl->Expand(m_conversationsItem);

				CTreeItemData * itemDataNew = new CTreeItemData(itemNewConversation, dialogInfo);
				m_treeCtrl->SetItemData(itemNewConversation, itemDataNew);
			}else
			{
				itemNewConversation = finditemUser->GetId();
				m_treeCtrl->SetItemText(itemNewConversation, sDialogAccountString);
			}
		}

		BOOST_ASSERT (gRightWindow != NULL);
		if ((fromDialogInfo.get() != NULL && gRightWindow->isCurrentSelect(fromInfo)) ||
			(fromUserInfo.get() != NULL && gRightWindow->isCurrentSelect(fromInfo)))
		{
			gRightWindow->addMessage(conversationInfo);
		}else
		{
			m_treeCtrl->SetItemBold(itemNewConversation);
			theFrame->changeTaskBarIcon("unread.ico", gLangText.textHadUnreadMsgTip());
		}
	}
}
//
//void RelationshipPanel::OnBeginLabelEdit(wxTreeEvent& event)
//{
//   // wxLogMessage(wxT("OnBeginLabelEdit"));
//
//    // for testing, prevent this item's label editing
//    //wxTreeItemId itemId = event.GetItem();
//    //if ( IsTestItem(*m_treeCtrl, itemId) )
//    //{
//    //    wxMessageBox(wxT("You can't edit this item."));
//
//    //    event.Veto();
//    //}
//    //else if ( itemId == m_treeCtrl->GetRootItem() )
//    //{
//    //    // test that it is possible to change the text of the item being edited
//    //    m_treeCtrl->SetItemText(itemId, _T("Editing root item"));
//    //}
//}
//
//void RelationshipPanel::OnEndLabelEdit(wxTreeEvent& event)
//{
//	wxString label = event.GetLabel();
//
//	if (EditLableType == 1)
//	{
//		if (event.IsEditCancelled())
//			label = m_treeCtrl->GetItemText(event.GetItem());
//
//		m_newitems.insert(label.c_str(), event.GetItem());
//		CGroupInfo::pointer groupInfo = CGroupInfo::create(label.c_str(), CGroupInfo::GT_Normal);
//
//		wxTreeItemId parentItem = m_treeCtrl->GetItemParent(event.GetItem());
//		wxString s = m_treeCtrl->GetItemText(parentItem);
//		CTreeItemData * itemData = (CTreeItemData*)m_treeCtrl->GetItemData(parentItem);
//		if (itemData != 0)
//		{
//			groupInfo->parentgroup(itemData->groupInfo());
//		}
//		m_biwoo.createFriendGroup(groupInfo);
//	}else if (!event.IsEditCancelled())
//	{
//		// Update
//		CTreeItemData * itemData = (CTreeItemData*)m_treeCtrl->GetItemData(event.GetItem());
//		if (itemData != 0)
//		{
//			itemData->groupInfo()->name(label.c_str());
//			m_biwoo.groupChangeName(itemData->groupInfo()->groupid(), label.c_str());
//		}
//	}
//
//	EditLableType = 0;
//    //wxLogMessage(wxT("OnEndLabelEdit"));
//
//    // don't allow anything except letters in the labels
//    //if ( !event.GetLabel().IsWord() )
//    //{
//    //    wxMessageBox(wxT("The new label should be a single word."));
//
//    //    event.Veto();
//    //}
//}

void RelationshipPanel::OnSelChanged(wxTreeEvent& event)
{
	assert (gRightWindow != NULL);
	assert (m_treeCtrl != NULL);

	//event
    wxTreeItemId itemId = event.GetItem();
	CTreeItemData* itemData = (CTreeItemData *)m_treeCtrl->GetItemData(itemId);
	if (itemData == NULL) return;

	if (m_treeCtrl->IsBold(itemId))
	{
		m_treeCtrl->SetItemBold(itemId, false);
		if (!m_biwoo.hasUnread())
		{
			theFrame->changeTaskBarIcon("mainframe.ico");
		}
	}
	switch (itemData->type())
	{
	case CTreeItemData::TTDT_COMPANYINFO:
		{
			gRightWindow->setCurrentSelect(CFromInfo::create(itemData->companyInfo()));
		}break;
	case CTreeItemData::TTDT_COGROUPINFO:
		{
			gRightWindow->setCurrentSelect(CFromInfo::create(itemData->cogroupInfo()));
		}break;
	case CTreeItemData::TTDT_USERINFO:
		{
			CUserInfo::pointer fromUserInfo = m_biwoo.getUserInfo(itemData->userInfo()->getAccount());
			assert (fromUserInfo.get() != NULL);
			gRightWindow->setCurrentSelect(CFromInfo::create(fromUserInfo));

		}break;
	case CTreeItemData::TTDT_DIALOGINFO:
		{
			CDialogInfo::pointer dialogInfo = itemData->dialogInfo();
			assert (dialogInfo.get() != NULL);

			CFromInfo::pointer selFromInfo = CFromInfo::create(dialogInfo);
			if (!gRightWindow->isCurrentSelect(selFromInfo))
			{
				gRightWindow->setCurrentSelect(selFromInfo);

				CLockMap<std::string, CUserInfoPointer>::iterator iter;
				for (iter=dialogInfo->m_members.begin(); iter!=dialogInfo->m_members.end(); iter++)
				{
					if (iter->second->getAccount() != gMyAccount->getUserinfo()->getAccount())
					{
						gRightWindow->invitedUser(iter->second);
					}
				}
			}
		}break;
	default:
		break;
	}
}

//void RelationshipPanel::OnItemActivated(wxTreeEvent& event)
//{
//	if (m_treeCtrl == NULL) return;
//
//    // show some info about this item
//    wxTreeItemId itemId = event.GetItem();
//   // MyTreeItemData *item = (MyTreeItemData *)m_treeCtrl->GetItemData(itemId);
//
//    //if ( item != NULL )
//    {
//       // item->ShowInfo(this);
//    }
//
//    //wxLogMessage(wxT("OnItemActivated"));
//}


void RelationshipPanel::ShowContextMenu(const wxPoint& pos)
{
	wxTreeItemId nCurrentItem = m_treeCtrl->GetSelection();
	if (!nCurrentItem.IsOk()) return;

	CTreeItemData * currentItemData = (CTreeItemData*)m_treeCtrl->GetItemData(nCurrentItem);
	if (currentItemData == NULL) return;

	wxMenu menu;

	//menu.Append(Menu_NewRootGroup, _T("New Root &Group"), _T("New Root Group"));
	//menu.Append(Menu_Popup_Submenu, _T("&Submenu"), CreateDummyMenu(NULL));

	switch (currentItemData->type())
	{
	case CTreeItemData::TTDT_USERINFO:
		{
			CUserInfo::pointer userInfo = currentItemData->userInfo();
			CAccountConversation::pointer accountConversation = m_biwoo.getAccountConversations(CFromInfo::create(userInfo));
			assert (accountConversation.get() != NULL);

			if (!accountConversation->m_conversations.empty())
			{
				wxString command = wxString::Format("%s: Delete Message", userInfo->getUserName().c_str());
				menu.Append(Menu_Deletemessage, command, _T("Delete user conversations message"));
			}
		}break;
	case CTreeItemData::TTDT_DIALOGINFO:
		{
			CDialogInfo::pointer dialogInfo = currentItemData->dialogInfo();
			CAccountConversation::pointer accountConversation = m_biwoo.getAccountConversations(CFromInfo::create(dialogInfo));
			assert (accountConversation.get() != NULL);

			if (!accountConversation->m_conversations.empty())
			{
				wxString sDialogAccountString = getDialogInfoAccount(dialogInfo);
				wxString command = wxString::Format("%s: Delete Message", sDialogAccountString.c_str());
				menu.Append(Menu_Deletemessage, command, _T("Delete dialog conversations message"));
			}

		}break;
	default:
		break;
	}

	PopupMenu(&menu, pos.x, pos.y);
}

#if USE_CONTEXT_MENU
void RelationshipPanel::OnContextMenu(wxContextMenuEvent& event)
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

void RelationshipPanel::OnMenuDeleteMessage(wxCommandEvent& WXUNUSED(event))
{
	wxTreeItemId nCurrentItem = m_treeCtrl->GetSelection();
	if (!nCurrentItem.IsOk()) return;

	wxString sItemText = m_treeCtrl->GetItemText(nCurrentItem);

	CTreeItemData * currentItemData = (CTreeItemData*)m_treeCtrl->GetItemData(nCurrentItem);
	if (currentItemData == NULL) return;

	wxString sText = wxString::Format("Are you sure you want to delete the messages of this user.\n%s", sItemText.c_str());
	if (wxMessageBox(sText,	wxT("biwoo"), wxICON_QUESTION | wxYES_NO, this) != wxYES)
	{
		return;
	}

	CFromInfo::pointer fromInfo;
	if (currentItemData->type() == CTreeItemData::TTDT_USERINFO)
	{
		fromInfo = CFromInfo::create(currentItemData->userInfo());
		if (gRightWindow->isCurrentSelect(fromInfo))
		{
			gRightWindow->clearMessage();
		}
	}else if (currentItemData->type() == CTreeItemData::TTDT_DIALOGINFO)
	{
		fromInfo = CFromInfo::create(currentItemData->dialogInfo());
		if (gRightWindow->isCurrentSelect(fromInfo))
		{
			gRightWindow->clearMessage();
		}
	}else
	{
		assert (0);
	}
	m_biwoo.deleteUserConversation(fromInfo);
	m_treeCtrl->Delete(nCurrentItem);
}
//
//void RelationshipPanel::OnMenuNewRootGroup(wxCommandEvent& WXUNUSED(event))
//{
//	if (m_treeCtrl == NULL) return;
//
//	m_biwoo.setHandler((CbiwooHandler*)this);
//
//	wxTreeItemId item = m_treeCtrl->InsertItem(m_treeCtrl->GetRootItem(), -1, wxT("New Group"));
//	//m_treeCtrl->Expand(m_rootItem);
//	m_treeCtrl->EditLabel(item);
//	EditLableType = 1;
//}
//
//void RelationshipPanel::OnMenuNewChildGroup(wxCommandEvent& WXUNUSED(event))
//{
//	if (m_treeCtrl == NULL) return;
//
//	m_biwoo.setHandler((CbiwooHandler*)this);
//
//	wxTreeItemId nCurrentItem = m_treeCtrl->GetSelection();
//	wxTreeItemId item = m_treeCtrl->InsertItem(nCurrentItem, -1, wxT("New Child Group"));
//	m_treeCtrl->Expand(nCurrentItem);
//	m_treeCtrl->EditLabel(item);
//	EditLableType = 1;
//}
//
//void RelationshipPanel::OnMenuDeleteGroup(wxCommandEvent& WXUNUSED(event))
//{
//	if (m_treeCtrl == NULL) return;
//
//	m_biwoo.setHandler((CbiwooHandler*)this);
//
//	wxTreeItemId nCurrentItem = m_treeCtrl->GetSelection();
//
//	if (m_treeCtrl->ItemHasChildren(nCurrentItem))
//	{
//        wxMessageBox(wxT("Can not delete the item, when it has children item."));
//		return;
//	}
//
//	CTreeItemData * itemData = (CTreeItemData*)m_treeCtrl->GetItemData(nCurrentItem);
//	if (itemData != 0)
//	{
//		m_biwoo.deleteFriendGroup(itemData->groupInfo()->groupid());
//		m_cogitemdatas.find(itemData->groupInfo()->groupid(), true);
//	}
//
//	m_treeCtrl->Delete(nCurrentItem);
//}
	
void RelationshipPanel::onRegConfirm(void)
{
	m_bGetRecents = true;
}

void RelationshipPanel::onUserLogouted(CAccountInfo::pointer accountInfo, CbiwooHandler::LogoutType logoutType)
{
	m_treeCtrl->DeleteAllItems();
	clearDatas();

	if (gMyCoGroup == (CbiwooHandler*)this)
	{
		m_conversationsItem = m_treeCtrl->InsertItem(TVI_ROOT, -1, gLangText.textConversations(), 1);
		gRightWindow->reset();
	}else if (gMyCompany == (CbiwooHandler*)this)
	{
		m_allContactsItem = m_treeCtrl->InsertItem(TVI_ROOT, -1, gLangText.textAllContacts(), 1);
	}
}

wxString RelationshipPanel::getDialogInfoAccount(CDialogInfo::pointer dialogInfo) const
{
	BOOST_ASSERT (dialogInfo.get() != NULL);

	bool bGetFirstUsered = false;
	wxString sDialogAccountString;
	CLockMap<std::string, CUserInfoPointer>::iterator iter;
	for (iter=dialogInfo->m_members.begin(); iter!=dialogInfo->m_members.end(); iter++)
	{
		if (!bGetFirstUsered)
		{
			bGetFirstUsered = true;
		}else
		{
			sDialogAccountString.append(",");
		}
		sDialogAccountString.append(iter->second->getUserName());
	}

	return sDialogAccountString;
}

void RelationshipPanel::onDialogInvited(CDialogInfo::pointer dialogInfo, CUserInfo::pointer inviteUserInfo)
{
	BOOST_ASSERT (gRightWindow != NULL);
	BOOST_ASSERT (theFrame != NULL);

	wxString sDialogAccountString = getDialogInfoAccount(dialogInfo);
	wxTreeItemId itemNewConversation;
	CTreeItemData * finditemDialog = findChildItem(m_conversationsItem, dialogInfo);
	if (finditemDialog == NULL)
	{
		itemNewConversation = m_treeCtrl->InsertItem(m_conversationsItem, -1, sDialogAccountString, cOnlineImage);
		m_treeCtrl->Expand(m_conversationsItem);

		m_treeCtrl->SetItemData(itemNewConversation, new CTreeItemData(itemNewConversation, dialogInfo));
	}else
	{
		itemNewConversation = finditemDialog->GetId();
		m_treeCtrl->SetItemText(itemNewConversation, sDialogAccountString);
	}

	if (gRightWindow->isCurrentSelect(CFromInfo::create(dialogInfo)))
	{
		gRightWindow->invitedUser(inviteUserInfo);
	}else if (gRightWindow->isCurrentSelect(CFromInfo::create(inviteUserInfo)))
	{
		gRightWindow->setCurrentSelect(CFromInfo::create(dialogInfo));
		gRightWindow->invitedUser(inviteUserInfo);
	}else
	{
		m_treeCtrl->SetItemBold(itemNewConversation);
		theFrame->changeTaskBarIcon("unread.ico", gLangText.textHadUnreadMsgTip());
	}

}

void RelationshipPanel::onGroupInfo(bool createReturn, CGroupInfo::pointer groupInfo)
{
	BOOST_ASSERT (groupInfo.get() != 0);
	if (m_treeCtrl == NULL) return;

	if (createReturn)
	{
		wxTreeItemId itemFind;
		if (m_newitems.find(groupInfo->name(), itemFind, true))
		{
			CTreeItemData * itemData = new CTreeItemData(itemFind, groupInfo);
			m_cogitemdatas.insert(groupInfo->groupid(), itemData);
			m_treeCtrl->SetItemData(itemFind, itemData);
		}

	}else
	{
		CTreeItemData * parentItemData = m_cogitemdatas.find(groupInfo->parentgroupid());

		CTreeItemData * itemData = m_cogitemdatas.find(groupInfo->groupid());
		if (itemData == NULL)
		{
			wxTreeItemId itemNew;
			if (parentItemData != NULL)
			{
				itemNew = m_treeCtrl->InsertItem(parentItemData->GetId(), -1, groupInfo->name(), 1);
				m_treeCtrl->Expand(parentItemData->GetId());
			}else
			{
				itemNew = m_treeCtrl->InsertItem(TVI_ROOT, -1, groupInfo->name(), 1);
				//m_treeCtrl->Expand(m_rootItem);
			}

			itemData = new CTreeItemData(itemNew, groupInfo);
			m_cogitemdatas.insert(groupInfo->groupid(), itemData);
			m_treeCtrl->SetItemData(itemNew, itemData);
		}

	}
}

void RelationshipPanel::onCompanyInfo(CCompanyInfo::pointer companyInfo)
{
	BOOST_ASSERT (companyInfo.get() != 0);
	if (m_treeCtrl == NULL) return;

	CTreeItemData * itemData = m_coitemdatas.find(companyInfo->id());
	if (itemData == NULL)
	{
		wxTreeItemId itemNew;
		itemNew = m_treeCtrl->InsertItem(TVI_ROOT, -1, companyInfo->name(), 1);
		//itemNew = m_treeCtrl->InsertItem(m_treeCtrl->GetRootItem(), -1, companyInfo->name(), 1);
		//m_treeCtrl->Expand(m_rootItem);

		itemData = new CTreeItemData(itemNew, companyInfo);
		m_coitemdatas.insert(companyInfo->id(), itemData);
		m_treeCtrl->SetItemData(itemNew, itemData);
	}

}

void RelationshipPanel::onCoGroupInfo(CCoGroupInfo::pointer cogroupInfo, bool isMyCoGroup)
{
	BOOST_ASSERT (cogroupInfo.get() != 0);
	if (m_treeCtrl == NULL) return;

	CCompanyInfo::pointer companyInfo = cogroupInfo->companyInfo();
	CTreeItemData * companyItemData = m_coitemdatas.find(companyInfo->id());
	if (companyItemData == NULL) return;

	CTreeItemData * itemData = m_cogitemdatas.find(cogroupInfo->groupid());
	if (itemData == NULL)
	{
		CTreeItemData * parentItemData = m_cogitemdatas.find(cogroupInfo->parentgroupid());

		wxTreeItemId itemNew;
		if (parentItemData != NULL)
		{
			itemNew = m_treeCtrl->InsertItem(parentItemData->GetId(), -1, cogroupInfo->name(), -1);
			m_treeCtrl->Expand(parentItemData->GetId());
		}else
		{
			itemNew = m_treeCtrl->InsertItem(companyItemData->GetId(), -1, cogroupInfo->name(), -1);
			m_treeCtrl->Expand(companyItemData->GetId());
		}

		itemData = new CTreeItemData(itemNew, cogroupInfo);
		m_cogitemdatas.insert(cogroupInfo->groupid(), itemData);
		m_treeCtrl->SetItemData(itemNew, itemData);
	}
}

CTreeItemData * RelationshipPanel::findChildItem(wxTreeItemId itemId, CUserInfoPointer userInfo)
{
	BOOST_ASSERT (itemId.IsOk());
	BOOST_ASSERT (userInfo.get() != 0);

	wxTreeItemIdValue cookie;
	wxTreeItemId childItem = m_treeCtrl->GetFirstChild(itemId, cookie);
	while (childItem.IsOk())
	{
		CTreeItemData * childItemData = (CTreeItemData*)m_treeCtrl->GetItemData(childItem);
		CUserInfoPointer childUserInfo = childItemData->userInfo();
		if (childUserInfo.get() == userInfo.get())
		//if (childUserInfo.get() != 0 && childUserInfo->getAccount() == userInfo->getAccount())
		{
			return childItemData;
		}

		childItem = m_treeCtrl->GetNextChild(itemId, cookie);
	}

	return NULL;
}

CTreeItemData * RelationshipPanel::findChildItem(wxTreeItemId itemId, CDialogInfo::pointer dialogInfo)
{
	BOOST_ASSERT (itemId.IsOk());
	BOOST_ASSERT (dialogInfo.get() != 0);

	wxTreeItemIdValue cookie;
	wxTreeItemId childItem = m_treeCtrl->GetFirstChild(itemId, cookie);
	while (childItem.IsOk())
	{
		CTreeItemData * childItemData = (CTreeItemData*)m_treeCtrl->GetItemData(childItem);
		if (childItemData->type() == CTreeItemData::TTDT_DIALOGINFO)
		{
			CDialogInfo::pointer childDialogInfo = childItemData->dialogInfo();
			if (childDialogInfo->dialogId() == dialogInfo->dialogId())
			{
				return childItemData;
			}
		}

		childItem = m_treeCtrl->GetNextChild(itemId, cookie);
	}

	return NULL;
}

CTreeItemData * RelationshipPanel::findRootItem(wxTreeItemId itemId)
{
	BOOST_ASSERT (itemId.IsOk());
	wxTreeItemId itemRoot = itemId;

	wxTreeItemId itemParent = m_treeCtrl->GetItemParent(itemId);
	while (itemParent.IsOk() && itemParent != TVI_ROOT)
	{
		itemRoot = itemParent;
		itemParent = m_treeCtrl->GetItemParent(itemParent);
	}
	return (CTreeItemData*)m_treeCtrl->GetItemData(itemRoot);
}


void RelationshipPanel::onCoGroupUser(CCoGroupInfo::pointer cogroupInfo, CUserInfoPointer userInfo, bool isMyCoGroupUser)
{
	BOOST_ASSERT (cogroupInfo.get() != 0);
	BOOST_ASSERT (userInfo.get() != 0);
	if (m_treeCtrl == NULL) return;

	if (gMyCompany == (CbiwooHandler*)this)
	{
		if (!m_users.exist((unsigned long)userInfo.get()))
		{
			m_users.insert((unsigned long)userInfo.get(), userInfo);

			wxString sItem = wxString::Format("%s(%s)", userInfo->getUserName().c_str(), userInfo->getAccount().c_str());
			wxTreeItemId itemNew = m_treeCtrl->InsertItem(m_allContactsItem, -1, sItem, -1);
			CTreeItemData * itemDataNew = new CTreeItemData(itemNew, userInfo);
			m_treeCtrl->SetItemData(itemNew, itemDataNew);
		}
	}

	CCompanyInfo::pointer companyInfo = cogroupInfo->companyInfo();
	CTreeItemData * companyItemData = m_coitemdatas.find(companyInfo->id());
	if (companyItemData == NULL) return;

	CTreeItemData * cogroupItemData = m_cogitemdatas.find(cogroupInfo->groupid());
	if (cogroupItemData == NULL)
	{
		return;
	}

	CTreeItemData * finditemUser = findChildItem(cogroupItemData->GetId(), userInfo);
	if (finditemUser != NULL && finditemUser->userInfo().get() != 0)
	{
		return;
	}

	updateOnlineCount(CFromInfo::create(cogroupInfo));
	updateOnlineCount(CFromInfo::create(companyInfo));

	wxTreeItemId itemNew;
	itemNew = m_treeCtrl->InsertItem(cogroupItemData->GetId(), -1, userInfo->getUserName(), getUserLineStateImage(userInfo->getLineState()));
	m_treeCtrl->Expand(cogroupItemData->GetId());

	CTreeItemData * itemDataNew = new CTreeItemData(itemNew, userInfo);
	m_treeCtrl->SetItemData(itemNew, itemDataNew);
}

void RelationshipPanel::updateOnlineCount(CFromInfo::pointer fromInfo)
{
	BOOST_ASSERT (fromInfo.get() != 0);
	if (m_treeCtrl == NULL) return;

	switch (fromInfo->fromType())
	{
	case CFromInfo::FromCoGroup:
		{
			CCoGroupInfo::pointer cogroupInfo = fromInfo->fromCoGroup();
			CCompanyInfo::pointer companyInfo = cogroupInfo->companyInfo();

			while (cogroupInfo.get() != NULL)
			{
				CTreeItemData * cogroupItemData = m_cogitemdatas.find(cogroupInfo->groupid());
				if (cogroupItemData == NULL)
				{
					break;
				}

				wxString cogroupString = wxString::Format("%s (%d/%d)", cogroupInfo->name().c_str(), cogroupInfo->onlineCount(), companyInfo->getCoGroupUser(cogroupInfo->groupid()));
				m_treeCtrl->SetItemText(cogroupItemData->GetId(), cogroupString);
				cogroupInfo = cogroupInfo->parentgroup();
			}
		}break;
	case CFromInfo::FromCompany:
		{
			CCompanyInfo::pointer companyInfo = fromInfo->fromCompany();

			CTreeItemData * companyItemData = m_coitemdatas.find(companyInfo->id());
			if (companyItemData == NULL) return;

			wxString companyString = wxString::Format("%s (%d/%d)", companyInfo->name().c_str(), companyInfo->onlineCount(), companyInfo->m_userinfos.size());
			m_treeCtrl->SetItemText(companyItemData->GetId(), companyString);
		}break;
	default:
		break;
	}
}

void RelationshipPanel::onUserLogined(CCoGroupInfo::pointer cogroupInfo, CUserInfo::pointer userInfo)
{
	BOOST_ASSERT (cogroupInfo.get() != 0);
	BOOST_ASSERT (userInfo.get() != 0);
	if (m_treeCtrl == NULL) return;

	CCompanyInfo::pointer companyInfo = cogroupInfo->companyInfo();
	CTreeItemData * companyItemData = m_coitemdatas.find(companyInfo->id());
	if (companyItemData == NULL) return;

	CTreeItemData * cogroupItemData = m_cogitemdatas.find(cogroupInfo->groupid());
	if (cogroupItemData == NULL) return;

	CTreeItemData * finditemUser = findChildItem(cogroupItemData->GetId(), userInfo);
	if (finditemUser == NULL || finditemUser->userInfo().get() == NULL)
	{
		return;
	}
	m_treeCtrl->SetItemImage(finditemUser->GetId(), cOnlineImage);

	updateOnlineCount(CFromInfo::create(cogroupInfo));
	updateOnlineCount(CFromInfo::create(companyInfo));

	finditemUser = NULL;
	if (m_allContactsItem.IsOk())
	{
		finditemUser = findChildItem(m_allContactsItem, userInfo);
	}else if (m_conversationsItem.IsOk())
	{
		finditemUser = findChildItem(m_conversationsItem, userInfo);
	}
	if (finditemUser != NULL && finditemUser->userInfo().get() != 0)
	{
		m_treeCtrl->SetItemImage(finditemUser->GetId(), cOnlineImage);
	}
}

void RelationshipPanel::onUserLogouted(CCoGroupInfo::pointer cogroupInfo, CUserInfo::pointer userInfo)
{
	BOOST_ASSERT (cogroupInfo.get() != 0);
	BOOST_ASSERT (userInfo.get() != 0);
	if (m_treeCtrl == NULL) return;

	CCompanyInfo::pointer companyInfo = cogroupInfo->companyInfo();
	CTreeItemData * companyItemData = m_coitemdatas.find(companyInfo->id());
	if (companyItemData == NULL) return;

	CTreeItemData * cogroupItemData = m_cogitemdatas.find(cogroupInfo->groupid());
	if (cogroupItemData == NULL)
	{
		return;
	}

	CTreeItemData * finditemUser = findChildItem(cogroupItemData->GetId(), userInfo);
	if (finditemUser == NULL || finditemUser->userInfo().get() == NULL)
	{
		return;
	}
	m_treeCtrl->SetItemImage(finditemUser->GetId(), cOfflineImage);

	updateOnlineCount(CFromInfo::create(cogroupInfo));
	updateOnlineCount(CFromInfo::create(companyInfo));

	finditemUser = NULL;
	if (m_allContactsItem.IsOk())
	{
		finditemUser = findChildItem(m_allContactsItem, userInfo);
	}else if (m_conversationsItem.IsOk())
	{
		finditemUser = findChildItem(m_conversationsItem, userInfo);
	}
	if (finditemUser != NULL && finditemUser->userInfo().get() != 0)
	{
		m_treeCtrl->SetItemImage(finditemUser->GetId(), cOfflineImage);
	}
}

//void RelationshipPanel::onSendMessage(CCoGroupInfo::pointer cogroupInfo, CConversationInfo::pointer conversationInfo)
//{
//	BOOST_ASSERT (companyInfo.get() != 0);
//	BOOST_ASSERT (cogroupInfo.get() != 0);
//	BOOST_ASSERT (conversationInfo.get() != 0);
//	if (m_treeCtrl == NULL) return;
//
//	CTreeItemData * companyItemData = m_coitemdatas.find(companyInfo->id());
//	if (companyItemData == NULL) return;
//
//	CTreeItemData * itemData = m_cogitemdatas.find(cogroupInfo->groupid());
//	if (itemData == NULL)
//	{
//		return;
//	}
//
//	CTreeItemData * finditemUser = findChildItem(itemData->GetId(), conversationInfo->sayAccount());
//	if (finditemUser == NULL || finditemUser->userInfo().get() == NULL)
//	{
//		return;
//	}
//
//	CUserInfo::pointer userInfo = conversationInfo->sayAccount();
//	if (gMyCoGroup == (CbiwooHandler*)this)
//	{
//		BOOST_ASSERT (m_conversationsItem.IsOk());
//		wxTreeItemId itemNew;
//		itemNew = m_treeCtrl->InsertItem(m_conversationsItem, -1, userInfo->getUserName(), getUserLineStateImage(userInfo->getLineState()));
//		m_treeCtrl->Expand(m_conversationsItem);
//
//		CTreeItemData * itemDataNew = new CTreeItemData(itemNew, userInfo);
//		m_treeCtrl->SetItemData(itemNew, itemDataNew);
//
//	}
//
//	BOOST_ASSERT (gRightWindow != NULL);
//	if (gRightWindow->getCurrentUser().get() == userInfo.get())
//	{
//		gRightWindow->addMessage(conversationInfo);
//	}else
//	{
//		m_treeCtrl->SetItemImage(finditemUser->GetId(), cNewMsgImage);
//	}
//}

//void RelationshipPanel::onSendFileRequest(CUserInfo::pointer fromUserInfo, long mid, const tstring & sFilename, long nFilesize)
//{
//	BOOST_ASSERT (fromUserInfo.get() != 0);
//
//	// ??? for test
//	//m_biwoo.msgRejectFile(fromUserInfo, mid);
//
//	BOOST_ASSERT (gRightWindow != NULL);
//	if (gRightWindow->getCurrentUser().get() == fromUserInfo.get())
//	{
//		gRightWindow->sendFileRequest(fromUserInfo, mid, sFilename, nFilesize);
//	}else
//	{
//		//m_treeCtrl->SetItemBold(finditemUser->GetId());
//	}
//}

