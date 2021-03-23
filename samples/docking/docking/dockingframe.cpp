#include <wx/docking/dockingframe.h>
#include <wx/docking/dockingpanel.h>


using namespace std;

wxIMPLEMENT_DYNAMIC_CLASS(wxDockingFrame, wxFrame);

wxBEGIN_EVENT_TABLE(wxDockingFrame, wxFrame)
	EVT_SIZE(wxDockingFrame::OnSize)
	EVT_CHILD_FOCUS(wxDockingFrame::OnChildFocus)
wxEND_EVENT_TABLE()

wxDockingFrame::wxDockingFrame()
: m_rootPanel(nullptr)
, m_activePanel(nullptr)
{
}

wxDockingFrame::wxDockingFrame(wxWindow *parent,
	wxWindowID id,
	const wxString &title,
	const wxPoint &pos,
	const wxSize &size,
	long style,
	const wxString &name)
: wxFrame()
, m_rootPanel(nullptr)
, m_activePanel(nullptr)
{
	Create(parent, id, title, pos, size, style, name);
}

wxDockingFrame::~wxDockingFrame()
{
}

bool wxDockingFrame::Create(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
{
	if (!wxFrame::Create(parent, id, title, pos, size, style, name))
		return false;

	m_rootPanel = new wxDockingPanel(this);
	m_activePanel = m_rootPanel;
	m_rootPanel->SetSize(GetClientSize());

	return true;
}

void wxDockingFrame::OnSize(wxSizeEvent &event)
{
	wxSize sz = GetClientSize();
	m_rootPanel->SetSize(sz);
	event.Skip();
}

void wxDockingFrame::OnChildFocus(wxChildFocusEvent &event)
{
	wxObject * o = event.GetEventObject();
	event.Skip();
}

wxDockingPanel *wxDockingFrame::TabifyPanel(wxWindow *panel, wxString const &title, wxDockingPanel *dockingPanel)
{
	wxWindow *existingPanel = dockingPanel->getUserPanel();
	if (existingPanel)
	{
		// We have to tabify here if the dockingPanel is already in use.
	}
	else
	{
		wxSize sz = dockingPanel->GetClientSize();
		dockingPanel->setUserPanel(panel);
		panel->Reparent(dockingPanel);
		panel->SetSize(sz);
	}

	return dockingPanel;
}

wxDockingPanel *wxDockingFrame::SplitPanel(wxWindow *panel, wxString const &title, wxDirection direction, wxDockingPanel *dockingPanel)
{
	wxSize sz = dockingPanel->GetClientSize();

	wxDockingPanel *dp1 = nullptr;
	wxDockingPanel *dp2 = nullptr;
	wxDockingPanel *result = nullptr;

	wxSplitterWindow *splitter = new wxSplitterWindow(dockingPanel, wxID_ANY);
	splitter->SetSashGravity(1.0);

	wxSplitterWindow *existingSplitter = dockingPanel->getSplitter();
	wxWindow *existingPanel = dockingPanel->getUserPanel();
	if (existingPanel)
	{
		dp1 = new wxDockingPanel(splitter, wxID_ANY, title);
		dp1->setUserPanel(existingPanel);
		existingPanel->Reparent(dp1);
	}
	else if (existingSplitter)
	{
		dp1 = new wxDockingPanel(splitter, wxID_ANY, title);
		dp1->setSplitter(existingSplitter);
		existingSplitter->Reparent(dp1);
	}
	else
	{
		wxNotebook *existingNotebook = dockingPanel->getNotebook();
		dp1 = new wxDockingPanel(splitter, wxID_ANY, title);
		dp1->setNotebook(existingNotebook);
		existingNotebook->Reparent(dp1);
	}

	dp2 = new wxDockingPanel(splitter, wxID_ANY, title);
	dp2->setUserPanel(panel);
	panel->Reparent(dp2);
	result = dp2;

	if (direction == wxLEFT || direction == wxRIGHT)
	{
		if (direction == wxLEFT)
			swap(dp1, dp2);

		uint32_t pos = sz.x / 2;
		splitter->SplitVertically(dp1, dp2, pos);
	}
	else
	{
		if (direction == wxUP)
			swap(dp1, dp2);

		uint32_t pos = sz.y / 2;
		splitter->SplitHorizontally(dp1, dp2, pos);
	}
	dockingPanel->setSplitter(splitter);
	splitter->SetSize(sz);

	return result;
}

wxDockingPanel *wxDockingFrame::AddPanel(wxWindow *panel, const wxString &title, wxDirection direction, wxDockingPanel *dockingPanel)
{
	if (panel == nullptr)
		return nullptr;

	if (!dockingPanel)
		dockingPanel = m_rootPanel;

	if(dockingPanel->GetChildren().empty())
		direction = wxCENTRAL;

	if (direction == wxCENTRAL)
		return TabifyPanel(panel, title, dockingPanel);

	return SplitPanel(panel, title, direction, dockingPanel);
}
