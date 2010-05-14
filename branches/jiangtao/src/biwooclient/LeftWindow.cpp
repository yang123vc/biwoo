#ifdef WIN32
#define _WIN32_WINNT 0x0600
#include "winsock2.h"
# include <windows.h>
#endif // WIN32

#include "LeftWindow.h"

RSDataPanel * gRSDatapancle = NULL;

BEGIN_EVENT_TABLE(LeftWindow, wxPanel)

EVT_SIZE    (           LeftWindow::OnSize)

END_EVENT_TABLE()

LeftWindow::LeftWindow(wxWindow * parent)
: wxPanel(parent)
, m_relationship(NULL)
, m_datapanel(NULL)

{
    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, GetClientSize(), wxSP_NOBORDER);

    m_relationship = new RelationshipWindow(m_splitter);
    m_datapanel = new RSDataPanel(m_splitter);
	gRSDatapancle = m_datapanel;

	m_splitter->SetMinimumPaneSize(200);
	m_splitter->SetBorderSize(0);
	m_splitter->SplitHorizontally(m_relationship, m_datapanel, 600);
}

LeftWindow::~LeftWindow(void)
{
	delete m_splitter;
	m_datapanel = NULL;
}

void LeftWindow::OnSize(wxSizeEvent& event)
{
	if (m_splitter)
		m_splitter->SetSize(event.GetSize());
}
