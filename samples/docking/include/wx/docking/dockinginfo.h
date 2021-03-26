#pragma once

#include <wx/defs.h>

#if wxUSE_DOCKING

#include <wx/string.h>

class wxDockingPanel;
class wxDockingFrame;

class wxDockingInfo
{
	friend wxDockingFrame;

public:
	wxDockingInfo(wxString const &title = "")
	: m_title(title)
	, m_dockingPanel(nullptr)
	, m_direction(wxCENTRAL)
	, m_horizontal(true)
	{
	}

	~wxDockingInfo()
	{
	}

	wxDockingInfo &title(wxString const &title) { m_title = title; return *this; }
	wxDockingInfo &dock(wxDockingPanel *dockingPanel) { m_dockingPanel = dockingPanel; return *this; }

	wxDockingInfo &center(void) { m_direction = wxCENTRAL; return *this; }
	bool isCenter(void) { return m_direction == wxCENTRAL; }
	wxDockingInfo &up(void) { m_direction = wxUP; return *this; }
	bool isUp(void) { return m_direction == wxUP; }
	wxDockingInfo &down(void) { m_direction = wxDOWN; return *this; }
	bool isDown(void) { return m_direction == wxDOWN; }
	wxDockingInfo &left(void) { m_direction = wxLEFT; return *this; }
	bool isLeft(void) { return m_direction == wxLEFT; }
	wxDockingInfo &right(void) { m_direction = wxRIGHT; return *this; }
	bool isRight(void) { return m_direction == wxRIGHT; }
	wxDockingInfo &top(void) { m_direction = wxTOP; return *this; }
	bool isTop(void) { return m_direction == wxTOP; }
	wxDockingInfo &bottom(void) { m_direction = wxBOTTOM; return *this; }
	bool isBottom(void) { return m_direction == wxBOTTOM; }

	// Toolbars
	wxDockingInfo &horizontal(bool top = true) { m_horizontal = true; (top) ? m_direction = wxTOP : m_direction = wxBOTTOM; return *this; }
	bool isHorizontal(void) const { return m_horizontal; }

	wxDockingInfo &vertical(bool left = true) { m_horizontal = false; (left) ? m_direction = wxLEFT : m_direction = wxRIGHT; return *this; }
	bool isVertical(void) const { return !m_horizontal; }

private:
	wxString m_title;
	wxDockingPanel *m_dockingPanel;

	// Splitting
	wxDirection m_direction;

	// Toolbars
	bool m_horizontal:1;
};

#endif // wxUSE_DOCKING
