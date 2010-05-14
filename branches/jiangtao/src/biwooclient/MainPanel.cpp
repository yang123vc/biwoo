#include "MainPanel.h"

#include "wx/sizer.h"
#include "wx/gbsizer.h"
#include "wx/statline.h"
#include "wx/notebook.h"
#include "wx/spinctrl.h"

BEGIN_EVENT_TABLE(MainPanel, wxPanel)
/*    EVT_MENU(wxID_EXIT,  MyFrame::OnQuit)
EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
EVT_MENU(2, MyFrame::OnNewDialog)
EVT_MENU(3, MyFrame::OnNewDialog2)
*/
EVT_SIZE    (           MainPanel::OnSize)

    //EVT_TREE_BEGIN_LABEL_EDIT(TreeTest_Ctrl, MainPanel::OnBeginLabelEdit)
    //EVT_TREE_END_LABEL_EDIT(TreeTest_Ctrl, MainPanel::OnEndLabelEdit)
    //EVT_TREE_SEL_CHANGED(TreeTest_Ctrl, MainPanel::OnSelChanged)

END_EVENT_TABLE()

MainPanel::MainPanel(wxWindow *parent, wxWindowID winid)
: wxPanel(parent, winid)
//, m_treeCtrl(NULL)

{
	//long style = wxTR_HAS_BUTTONS|wxTR_EDIT_LABELS | wxTR_LINES_AT_ROOT | wxTR_SINGLE | wxTR_EXTENDED;
 //   m_treeCtrl = new wxTreeCtrl(this, TreeTest_Ctrl,
 //                               wxPoint(0,0), wxDefaultSize,
 //                               style);

	//wxTreeItemId nItem = m_treeCtrl->InsertItem(m_treeCtrl->GetRootItem(), -1, wxT("root item"));
	//m_treeCtrl->InsertItem(nItem, -1, wxT("1nd item"));
	//m_treeCtrl->InsertItem(nItem, -1, wxT("2nd item"));
	//m_treeCtrl->ExpandAll();
}

MainPanel::~MainPanel(void)
{
}

void MainPanel::OnSize(wxSizeEvent& event)
{
	//if (m_treeCtrl)
	//	m_treeCtrl->SetSize(200, event.GetSize().GetHeight());
}

//
//void MainPanel::OnBeginLabelEdit(wxTreeEvent& event)
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
//void MainPanel::OnEndLabelEdit(wxTreeEvent& event)
//{
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
//
//void MainPanel::OnSelChanged(wxTreeEvent& event)
//{
//
//	//event
//
//}
