#if wxUSE_DOCKING

#include <wx/docking/dockingpanel.h>

wxIMPLEMENT_DYNAMIC_CLASS(wxDockingPanel, wxPanel);

wxBEGIN_EVENT_TABLE(wxDockingPanel, wxPanel)
	EVT_SIZE(wxDockingPanel::OnSize)
wxEND_EVENT_TABLE()

wxDockingPanel::wxDockingPanel()
{
	init();
}

wxDockingPanel::wxDockingPanel(wxWindow *parent,
	wxWindowID id,
	wxString const &title,
	const wxPoint &pos,
	const wxSize &size,
	long style,
	const wxString &name)
: wxPanel(parent, id, pos, size, style, name)
{
	init(title);
}

wxDockingPanel::~wxDockingPanel()
{
}

void wxDockingPanel::init(wxString const &title)
{
	m_title = title;
	m_panel = nullptr;
	initFlags();
}

void wxDockingPanel::OnSize(wxSizeEvent &event)
{
	wxWindowList &children = GetChildren();

	for(wxWindow *w : children)
		w->SetSize(GetClientSize());

	event.Skip();
}

#endif // wxUSE_DOCKING
