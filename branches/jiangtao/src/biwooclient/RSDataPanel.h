// RSDataPanel.h file here
#ifndef __RSDataPanel_h__
#define __RSDataPanel_h__

#include "wx/wxprec.h"
#include "wx/treectrl.h"
#include "incdef.h"


class RSDataPanel
	//: public wxScrolledWindow
	: public wxPanel
{
private:
	wxTreeCtrl * m_treeCtrl;
	wxTreeItemId m_rootItem;

public:
	RSDataPanel(wxWindow * parent);
	~RSDataPanel(void);

	void initSample(void);

private:
    void ShowContextMenu(const wxPoint& pos);

	void OnSize(wxSizeEvent& event);
	void OnMenuNewRootGroup(wxCommandEvent& WXUNUSED(event));
	void OnMenuNewChildGroup(wxCommandEvent& WXUNUSED(event));

	void OnBeginLabelEdit(wxTreeEvent& event);
    void OnEndLabelEdit(wxTreeEvent& event);
    void OnSelChanged(wxTreeEvent& event);

#if USE_CONTEXT_MENU
    void OnContextMenu(wxContextMenuEvent& event);
#else
    void OnRightUp(wxMouseEvent& event)
        { ShowContextMenu(event.GetPosition()); }
#endif // USE_CONTEXT_MENU
    DECLARE_EVENT_TABLE()

};

// control ids
enum
{
    DataTree_Ctrl = 1000
};

#endif // __RSDataPanel_h__
