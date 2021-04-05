#if wxUSE_DOCKING

#include <wx/docking/dockinginfo.h>
#include <wx/notebook.h>

wxDockingInfo::wxDockingInfo(wxString const &title)
: m_title(title)
, m_dockingPanel(nullptr)
, m_direction(wxCENTRAL)
, m_nbTabStyle(wxCENTRAL)
, m_horizontal(true)
, m_point(wxDefaultPosition)
, m_size(wxDefaultSize)
, m_showTab(false)
{
	// Override default settings if available
	if (m_default)
	{
		m_nbTabStyle = m_default->m_nbTabStyle;
		m_showTab = m_default->m_showTab;
	}
}

long wxDockingInfo::tabStyle(void) const
{
	switch (m_nbTabStyle)
	{
		case wxLEFT: return wxNB_LEFT;
		case wxRIGHT: return wxNB_RIGHT;
		case wxTOP: return wxNB_TOP;
		case wxBOTTOM: return wxNB_BOTTOM;
	}

	return 0;
}

#endif // wxUSE_DOCKING
