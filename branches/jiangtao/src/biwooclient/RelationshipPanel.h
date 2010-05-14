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

// RelationshipPanel.h file here
#ifndef __RelationshipPanel_h__
#define __RelationshipPanel_h__

#include "wx/wxprec.h"
#include "wx/treectrl.h"
#include "incdef.h"
#include "bcd.h"

class CTreeItemData
	: public wxTreeItemData
{
public:
	enum TreeItemDataType
	{
		TTDT_GROUPINFO	= 0x1
		, TTDT_COMPANYINFO
		, TTDT_COGROUPINFO
		, TTDT_USERINFO
		, TTDT_DIALOGINFO
		, TTDT_UNKNOWN	= 0xf
	};

	CTreeItemData(const wxTreeItemId& id, CGroupInfo::pointer groupInfo)
		: m_type(TTDT_GROUPINFO), m_groupInfo(groupInfo)
	{
		SetId(id);
	}
	CTreeItemData(const wxTreeItemId& id, CCompanyInfo::pointer companyInfo)
		: m_type(TTDT_COMPANYINFO), m_companyInfo(companyInfo)
	{
		SetId(id);
	}
	CTreeItemData(const wxTreeItemId& id, CCoGroupInfo::pointer cogroupInfo)
		: m_type(TTDT_COGROUPINFO), m_cogroupInfo(cogroupInfo)
	{
		SetId(id);
	}
	CTreeItemData(const wxTreeItemId& id, CUserInfoPointer userInfo)
		: m_type(TTDT_USERINFO), m_userInfo(userInfo)
	{
		SetId(id);
	}
	CTreeItemData(const wxTreeItemId& id, CDialogInfo::pointer dialogInfo)
		: m_type(TTDT_DIALOGINFO), m_dialogInfo(dialogInfo)
	{
		SetId(id);
	}

	TreeItemDataType type(void) const {return m_type;}

	CGroupInfo::pointer groupInfo(void) const {return m_groupInfo;}
	CCompanyInfo::pointer companyInfo(void) const {return m_companyInfo;}
	CCoGroupInfo::pointer cogroupInfo(void) const {return m_cogroupInfo;}
	CUserInfoPointer userInfo(void) const {return m_userInfo;}
	CDialogInfo::pointer dialogInfo(void) const {return m_dialogInfo;}

private:
	TreeItemDataType	m_type;
	CGroupInfo::pointer m_groupInfo;
	CCompanyInfo::pointer m_companyInfo;
	CCoGroupInfo::pointer m_cogroupInfo;
	CUserInfoPointer m_userInfo;
	CDialogInfo::pointer m_dialogInfo;
};

class RelationshipPanel
	: public wxPanel
	, public CbiwooHandler
{
private:
	wxImageList * m_imageList;
	wxTreeCtrl * m_treeCtrl;
	wxTreeItemId m_allContactsItem;
	wxTreeItemId m_conversationsItem;

	CLockMap<unsigned long, CUserInfo::pointer> m_users;

	CLockMap<tstring, wxTreeItemId> m_newitems;
	CLockMapPtr<unsigned int, CTreeItemData*> m_coitemdatas;
	CLockMapPtr<unsigned int, CTreeItemData*> m_cogitemdatas;

	bool m_bGetRecents;
	wxTimer m_timer;

public:
	RelationshipPanel(wxWindow * parent);
	~RelationshipPanel(void);

	void buildAllContacts(void);
	void buildConversations(void);
	void DoSearchUsers(const wxString & searchString);

private:
	CTreeItemData * findChildItem(wxTreeItemId itemId, CUserInfoPointer userInfo);
	CTreeItemData * findChildItem(wxTreeItemId itemId, CDialogInfo::pointer dialogInfo);
	CTreeItemData * findRootItem(wxTreeItemId itemId);

	wxString getDialogInfoAccount(CDialogInfo::pointer dialogInfo) const;

	void updateOnlineCount(CFromInfo::pointer fromInfo);

private:
	void clearDatas(void);

    void ShowContextMenu(const wxPoint& pos);

	void OnSize(wxSizeEvent& event);
	void OnMenuDeleteMessage(wxCommandEvent& WXUNUSED(event));

	//void OnMenuNewRootGroup(wxCommandEvent& WXUNUSED(event));
	//void OnMenuNewChildGroup(wxCommandEvent& WXUNUSED(event));
	//void OnMenuDeleteGroup(wxCommandEvent& WXUNUSED(event));

	//void OnBeginLabelEdit(wxTreeEvent& event);
	//void OnEndLabelEdit(wxTreeEvent& event);

    void OnSelChanged(wxTreeEvent& event);
	//void OnItemActivated(wxTreeEvent& event);

	void OnTimer(wxTimerEvent & event);

#if USE_CONTEXT_MENU
    void OnContextMenu(wxContextMenuEvent& event);
#else
    void OnRightUp(wxMouseEvent& event)
        { ShowContextMenu(event.GetPosition()); }
#endif // USE_CONTEXT_MENU
    DECLARE_EVENT_TABLE()

	// CbiwooHandler
	virtual void onRegConfirm(void);
	virtual void onUserLogouted(CAccountInfo::pointer accountInfo, CbiwooHandler::LogoutType logoutType);
	virtual void onGroupInfo(bool createReturn, CGroupInfo::pointer groupInfo);
	virtual void onCompanyInfo(CCompanyInfo::pointer companyInfo);
	virtual void onCoGroupInfo(CCoGroupInfo::pointer cogroupInfo, bool isMyCoGroup);
	virtual void onCoGroupUser(CCoGroupInfo::pointer cogroupInfo, CUserInfoPointer userInfo, bool isMyCoGroupUser);

	virtual void onUserLogined(CCoGroupInfo::pointer cogroupInfo, CUserInfo::pointer userInfo);
	virtual void onUserLogouted(CCoGroupInfo::pointer cogroupInfo, CUserInfo::pointer userInfo);

	virtual void onDialogInvited(CDialogInfo::pointer dialogInfo, CUserInfo::pointer inviteUserInfo);
};

#endif // __RelationshipPanel_h__
