// MainPanel.h file here
#ifndef __MainPanel_h__
#define __MainPanel_h__

#include "wx/wxprec.h"
#include "wx/treectrl.h"

#define USE_GENERIC_TREECTRL 0
#if USE_GENERIC_TREECTRL
#include "wx/generic/treectlg.h"
#ifndef wxTreeCtrl
#define wxTreeCtrl wxGenericTreeCtrl
#define sm_classwxTreeCtrl sm_classwxGenericTreeCtrl
#endif
#endif

class MainPanel
	: public wxPanel
{
private:
//	wxTreeCtrl * m_treeCtrl;

public:
	MainPanel(wxWindow *parent, wxWindowID winid = wxID_ANY);
	~MainPanel(void);

protected:
    // is this the test item which we use in several event handlers?
    bool IsTestItem(const wxTreeCtrl & ctrl, const wxTreeItemId& item) const
    {
        // the test item is the first child folder
        return ctrl.GetItemParent(item) == ctrl.GetRootItem() && !ctrl.GetPrevSibling(item);
    }
private:
	void OnSize(wxSizeEvent& event);

	//void OnBeginLabelEdit(wxTreeEvent& event);
 //   void OnEndLabelEdit(wxTreeEvent& event);
 //   void OnSelChanged(wxTreeEvent& event);

    DECLARE_EVENT_TABLE()
};


// control ids
//enum
//{
//    TreeTest_Ctrl = 1000
//};

#endif // __MainPanel_h__
