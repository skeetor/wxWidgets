#if wxUSE_DOCKING

#include <wx/object.h>
#include <wx/notebook.h>
#include <wx/splitter.h>

#include <wx/docking/dockinginfo.h>
#include <wx/docking/dockingutils.h>
#include <wx/docking/dockingstate.h>

static wxDockingInfo gDefaults;

wxDockingInfo::wxDockingInfo(wxString const &title)
: m_frame(nullptr)
, m_windowAtPoint(nullptr)
, m_window(static_cast<wxWindow *>(nullptr))
, m_title(title)
, m_direction(wxALL)
, m_orientation(wxNONE)
, m_size(wxSize(-1, -1))
, m_tabDirection(wxTOP)
, m_page((size_t)-1)
, m_activate(false)
, m_floating(false)
, m_placeholder(false)
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
		m_windowAtPoint = nullptr;
		m_title = "";
		m_window = static_cast<wxWindow *>(nullptr);
		m_direction = wxALL;
		m_orientation = wxNONE;
		m_position = wxPoint(-1, -1);
		m_size = wxSize(-1, -1);
		m_activate = false;
		m_floating = false;
		m_placeholder = false;
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

void wxDockingInfo::FromNotebook(wxPoint coordinates, wxDockingEntity &notebook)
{
	wxNotebook *nb = notebook.GetNotebook();
	SetWindow(notebook);
	if (!nb->GetPageCount())
	{
		SetPage(static_cast<size_t>(wxNOT_FOUND));
		return;
	}

	wxWindow *window = nb->GetPage(nb->GetSelection());
	if (window->GetScreenRect().Contains(coordinates))
	{
		// Mouse is on the page
		SetWindow(window);
		SetPage(nb->GetSelection());
		SetTabArea(false);
		SetOnTab(false);
		return;
	}

	wxRect openRect = wxDockingUtils::GetTabOpenArea(nb);
	wxPoint windowPoint = nb->ScreenToClient(coordinates);
	//wxDockingUtils::PaintRect(openRect, window);
	if (openRect.Contains(windowPoint))
	{
		// Mouse is in the free area next to the tabs
		SetPage(static_cast<size_t>(wxNOT_FOUND));
		SetTabArea(true);
		SetOnTab(false);
		return;
	}

	// Check if the mouse is over a tab.
	// There may be a small gap between the border and the tab area. HitTest would report this
	// as NOWHERE, but this means that the target hint will flicker back and forth when the
	// user moves over it, so we want to avoid it and thus treat this area also as part of
	// the tab, if applicable. To achieve this, we align the rectangle from the tab with the
	// border.

	size_t nearestTab = (size_t)wxNOT_FOUND;
	wxRect notebookRect = nb->GetRect();

	double distance = std::numeric_limits<double>::max();
	for (size_t i = 0; i < nb->GetPageCount(); i++)
	{
		wxRect pageRect = nb->GetPage(i)->GetRect();
		wxRect tab = wxDockingUtils::GetAlignedTabRect(nb, openRect, pageRect, i);
		//PaintHint(tab, false, panel);
		if (tab.Contains(coordinates))
		{
			SetPage(i);
			SetTabArea(true);
			SetOnTab(true);
			return;
		}

		double d = wxDockingUtils::RectDistance(tab, windowPoint);
		if (d < distance)
		{
			distance = d;
			nearestTab = i;
		}
	}

	SetPage(nearestTab);
	SetTabArea(true);
	SetOnTab(true);
}

void wxDockingInfo::FromSplitter(wxPoint coordinates, wxDockingEntity &splitter, wxDockingEntity &lastSelected)
{
	wxSplitterWindow *sp = splitter.GetSplitter();
	wxPoint windowPoint = sp->ScreenToClient(coordinates);
	if (wxDockingUtils::IsOnSash(sp, windowPoint))
	{
		SetWindow(lastSelected);
		return;
	}
}

bool wxDockingInfo::FromPoint(wxPoint coordinates, wxDockingFrame *frame, wxDockingEntity &lastSelected)
{
	Clear();

	wxWindow *w = wxFindWindowAtPoint(coordinates);

	if (!w)
		return false;

	return FromWindow(coordinates, frame, w, lastSelected);
}

bool wxDockingInfo::FromWindow(wxPoint coordinates, wxDockingFrame *frame, wxWindow *window, wxDockingEntity &lastSelected)
{
	Clear();

	SetWindowAtPoint(window);
	wxDockingEntity source(window);

	wxDockingState const &gs = wxDockingState::GetInstance();
	if (!window || !gs.IsKnownPanel(source))
		return false;

	if (source.GetType() == wxDOCKING_NOTEBOOK)
		FromNotebook(coordinates, source);
	else if (source.GetType() == wxDOCKING_SPLITTER)
		FromSplitter(coordinates, source, lastSelected);
	else
		SetWindow(window);

	SetFrame(wxDockingUtils::DockingFrameFromWindow(source));
	if (!GetFrame())
		SetFrame(frame);

	return true;
}

void wxDockingInfo::UpdateToParent()
{
	// If the panel is a user window, we have to find the parent to remove it from.
	wxDockingEntity panel = GetWindow();
	wxDockingEntity parent = panel.GetRawWindow()->GetParent();

	switch (parent.GetType())
	{
		//case wxDOCKING_FRAME:
		//	SetWindow(parent);
		//break;

		case wxDOCKING_SPLITTER:
			SetWindow(parent);
		break;

		case wxDOCKING_NOTEBOOK:
		{
			wxNotebook *nb = parent.GetNotebook();
			wxWindow *w = panel.GetRawWindow();
			size_t page = nb->FindPage(w);
			// The window is parented to the notebook, but not a page???
			wxCHECK_MSG(page != wxNOT_FOUND, (void)0, wxT("Panel not found as a page"));

			SetPage(page);
			SetOnTab(false);
			SetTabArea(false);
			SetWindow(parent);
		}
		break;

		default:
			wxCHECK_MSG(false, (void)0, wxT("Unknown parent paneltype"));
	}
}

#endif // wxUSE_DOCKING
