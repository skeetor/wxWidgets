#if wxUSE_DOCKING

#include <wx/object.h>
#include <wx/notebook.h>
#include <wx/splitter.h>

#include <wx/docking/dockinginfo.h>
#include <wx/docking/dockingutils.h>

static wxDockingInfo gDefaults;

wxDockingInfo::wxDockingInfo(wxString const &title)
: m_frame(nullptr)
, m_panel(static_cast<wxWindow *>(nullptr))
, m_window(static_cast<wxWindow *>(nullptr))
, m_title(title)
, m_direction(wxALL)
, m_orientation(wxNONE)
, m_size(wxSize(-1, -1))
, m_tabDirection(wxTOP)
, m_page((size_t)-1)
, m_activate(false)
, m_floating(false)
, m_forcePanel(false)
, m_forceSplit(false)
, m_tabArea(false)
, m_onTab(false)
{
	// Override default settings if available
	*this = gDefaults;
	m_title = title;
}

wxDockingInfo &wxDockingInfo::GetDefaults()
{
	return gDefaults;
}

void wxDockingInfo::Clear()
{
	if (this != &gDefaults)
	{
		*this = gDefaults;
		m_direction = wxALL;
	}
	else
	{
		m_frame = nullptr;
		m_panel = wxDockingEntity();
		m_title = "";
		m_window = static_cast<wxWindow *>(nullptr);
		m_direction = wxALL;
		m_orientation = wxNONE;
		m_position = wxPoint(-1, -1);
		m_size = wxSize(-1, -1);
		m_activate = false;
		m_floating = false;
		m_forcePanel = false;
		m_forceSplit = false;
		m_tabArea = false;
		m_onTab = false;
		m_tabDirection = wxTOP;
		m_page = (size_t)-1;
	}
}

wxDockingEntity wxDockingInfo::GetDockingEntity() const
{
	if (m_window)
		return m_window;

	if (m_panel)
		return m_panel;

	// TODO: Why can't a I pass a framepointer in place of a wxWindow (MSW)???
	wxDockingEntity p;
	p.SetFrame(m_frame);

	return p;
}

long wxDockingInfo::GetTabStyle() const
{
	switch (m_tabDirection)
	{
		case wxLEFT: return wxNB_LEFT;
		case wxRIGHT: return wxNB_RIGHT;
		case wxTOP: return wxNB_TOP;
		case wxBOTTOM: return wxNB_BOTTOM;
	}

	return 0;
}

wxDockingInfo &wxDockingInfo::SetOrientation(wxOrientation orientation)
{
	if (orientation != wxHORIZONTAL && orientation != wxVERTICAL)
		orientation = (wxOrientation)0;

	m_orientation = orientation;

	return *this;
}

bool wxDockingInfo::FromWindow(wxWindow *source, wxDockingFrame *frame)
{
	Clear();

	if (!source)
		return false;

	wxWindow *dockingSource = nullptr;

	// If the source window is not part of a docking panel, we can't do anything about it.
	wxDockingEntity p = wxDockingUtils::FindParentPanel(source, &dockingSource);
	if (!p)
		return false;

	SetWindow(dockingSource);
	SetPanel(p);

	wxDockingEntityType panelType = p.GetType();

	if (panelType == wxDOCKING_NOTEBOOK)
	{
		if (dockingSource)
		{
			wxNotebook *nb = p.GetNotebook();
			SetWindow(nb->GetCurrentPage());
			SetPage(nb->GetSelection());
		}
	}
	else if (panelType == wxDOCKING_SPLITTER && dockingSource)
	{
		wxSplitterWindow *sp = p.GetSplitter();
		SetTitle(sp->GetName());

		wxOrientation orientation = (sp->GetSplitMode() == wxSPLIT_HORIZONTAL) ? wxHORIZONTAL : wxVERTICAL;
		SetOrientation(orientation);
	}

	SetFrame(wxDockingUtils::DockingFrameFromWindow(source));
	if (!GetFrame())
		SetFrame(frame);

	return true;
}

#endif // wxUSE_DOCKING
