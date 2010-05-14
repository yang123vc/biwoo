#include "RSDataPanel.h"

BEGIN_EVENT_TABLE(RSDataPanel, wxPanel)

EVT_SIZE    (           RSDataPanel::OnSize)

    EVT_MENU(Menu_NewRootGroup, OnMenuNewRootGroup)
    EVT_MENU(Menu_NewChildGroup, OnMenuNewChildGroup)

    EVT_TREE_BEGIN_LABEL_EDIT(DataTree_Ctrl, RSDataPanel::OnBeginLabelEdit)
    EVT_TREE_END_LABEL_EDIT(DataTree_Ctrl, RSDataPanel::OnEndLabelEdit)
    EVT_TREE_SEL_CHANGED(DataTree_Ctrl, RSDataPanel::OnSelChanged)

#if USE_CONTEXT_MENU
    EVT_CONTEXT_MENU(OnContextMenu)
#else
    EVT_RIGHT_UP(OnRightUp)
#endif // USE_CONTEXT_MENU

END_EVENT_TABLE()

RSDataPanel::RSDataPanel(wxWindow * parent)
: wxPanel(parent)
, m_treeCtrl(NULL)

{
	long style = wxTR_HAS_BUTTONS|wxTR_EDIT_LABELS | wxTR_LINES_AT_ROOT | wxTR_SINGLE | wxTR_EXTENDED;
    m_treeCtrl = new wxTreeCtrl(this, DataTree_Ctrl,
                                wxPoint(0,0), wxDefaultSize,
                                style);

	m_rootItem = m_treeCtrl->InsertItem(m_treeCtrl->GetRootItem(), -1, wxT("Relationship Data"));
}

RSDataPanel::~RSDataPanel(void)
{
}

void RSDataPanel::OnSize(wxSizeEvent& event)
{
	if (m_treeCtrl)
		m_treeCtrl->SetSize(event.GetSize());
}


void RSDataPanel::initSample(void)
{
	if (m_treeCtrl == NULL) return;

	m_treeCtrl->DeleteChildren(m_rootItem);
	m_treeCtrl->InsertItem(m_rootItem, -1, wxT("root group1"));
	m_treeCtrl->InsertItem(m_rootItem, -1, wxT("root group2"));
	m_treeCtrl->Expand(m_rootItem);
}

void RSDataPanel::OnBeginLabelEdit(wxTreeEvent& event)
{
   // wxLogMessage(wxT("OnBeginLabelEdit"));

    // for testing, prevent this item's label editing
    //wxTreeItemId itemId = event.GetItem();
    //if ( IsTestItem(*m_treeCtrl, itemId) )
    //{
    //    wxMessageBox(wxT("You can't edit this item."));

    //    event.Veto();
    //}
    //else if ( itemId == m_treeCtrl->GetRootItem() )
    //{
    //    // test that it is possible to change the text of the item being edited
    //    m_treeCtrl->SetItemText(itemId, _T("Editing root item"));
    //}
}

void RSDataPanel::OnEndLabelEdit(wxTreeEvent& event)
{
    //wxLogMessage(wxT("OnEndLabelEdit"));

    // don't allow anything except letters in the labels
    //if ( !event.GetLabel().IsWord() )
    //{
    //    wxMessageBox(wxT("The new label should be a single word."));

    //    event.Veto();
    //}
}

void RSDataPanel::OnSelChanged(wxTreeEvent& event)
{

	//event

}


void RSDataPanel::ShowContextMenu(const wxPoint& pos)
{
    wxMenu menu;

    menu.Append(Menu_NewRootGroup, _T("New Root &Group"), _T("New Root Group"));
    menu.Append(Menu_NewChildGroup, _T("New &Child Group"), _T("New Child Group"));
    //menu.Append(Menu_Popup_Submenu, _T("&Submenu"), CreateDummyMenu(NULL));

    //menu.Delete(Menu_Popup_ToBeDeleted);
    //menu.Check(Menu_Popup_ToBeChecked, true);
    //menu.Enable(Menu_Popup_ToBeGreyed, false);

    PopupMenu(&menu, pos.x, pos.y);
}

#if USE_CONTEXT_MENU
void RSDataPanel::OnContextMenu(wxContextMenuEvent& event)
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

void RSDataPanel::OnMenuNewRootGroup(wxCommandEvent& WXUNUSED(event))
{
	if (m_treeCtrl == NULL) return;

	m_treeCtrl->InsertItem(m_rootItem, -1, wxT("root item"));

}

void RSDataPanel::OnMenuNewChildGroup(wxCommandEvent& WXUNUSED(event))
{
	if (m_treeCtrl == NULL) return;

	wxTreeItemId nCurrentItem = m_treeCtrl->GetSelection();
	m_treeCtrl->InsertItem(nCurrentItem, -1, wxT("child item"));
	m_treeCtrl->Expand(nCurrentItem);
}
