// LeftWindow.h file here
#ifndef __LeftWindow_h__
#define __LeftWindow_h__

#include "wx/wxprec.h"
#include "wx/splitter.h"
#include "RelationshipWindow.h"
#include "RSDataPanel.h"

class LeftWindow
	//: public wxScrolledWindow
	: public wxPanel
{
private:
	wxSplitterWindow * m_splitter;
	RelationshipWindow * m_relationship;
	RSDataPanel * m_datapanel;

public:
	LeftWindow(wxWindow * parent);
	~LeftWindow(void);

	//RSDataPanel * getDataPanel(void) {return m_datapanel;}

private:
	void OnSize(wxSizeEvent& event);

    DECLARE_EVENT_TABLE()

};

extern RSDataPanel * gRSDatapancle;

#endif // __LeftWindow_h__
