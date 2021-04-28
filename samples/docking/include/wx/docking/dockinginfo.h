#pragma once

#include <wx/defs.h>

#if wxUSE_DOCKING

#include <wx/string.h>
#include <wx/gdicmn.h>

class wxDockingPanel;
class wxDockingFrame;

class WXDLLIMPEXP_DOCKING wxDockingInfo
{
	friend wxDockingFrame;

public:
	wxDockingInfo(wxString const &title = "");
	~wxDockingInfo()
	{
	}

	/**
	 * Make this the default instance.
	 */
	wxDockingInfo &defaults(void) { m_default = this; return *this; }

	wxDockingInfo &title(wxString const &title) { m_title = title; return *this; }
	wxString const &title() const { return m_title; }
	wxDockingInfo &dock(wxDockingPanel *dockingPanel) { m_dockingPanel = dockingPanel; return *this; }
	wxDockingPanel *dock(void) const { return m_dockingPanel; }

	bool isActive(void) const { return m_activate; }
	void activate(bool activate = true) { m_activate = activate; }

	wxDockingInfo &center(void) { m_direction = wxCENTRAL; return *this; }
	bool isCenter(void) const { return m_direction == wxCENTRAL; }
	wxDockingInfo &up(void) { m_direction = wxUP; return *this; }
	bool isUp(void) const { return m_direction == wxUP; }
	wxDockingInfo &down(void) { m_direction = wxDOWN; return *this; }
	bool isDown(void) const { return m_direction == wxDOWN; }
	wxDockingInfo &left(void) { m_direction = wxLEFT; return *this; }
	bool isLeft(void) const { return m_direction == wxLEFT; }
	wxDockingInfo &right(void) { m_direction = wxRIGHT; return *this; }
	bool isRight(void) const { return m_direction == wxRIGHT; }
	wxDockingInfo &top(void) { m_direction = wxTOP; return *this; }
	bool isTop(void) const { return m_direction == wxTOP; }
	wxDockingInfo &bottom(void) { m_direction = wxBOTTOM; return *this; }
	bool isBottom(void) const { return m_direction == wxBOTTOM; }
	wxDirection direction(void) const { return m_direction; }

	wxDockingInfo &tabstyleLeft(void) { m_nbTabStyle = wxLEFT; return *this; }
	bool isTabstyleLeft(void) const { return m_nbTabStyle == wxLEFT; }
	wxDockingInfo &tabstyleRight(void) { m_nbTabStyle = wxRIGHT; return *this; }
	bool isTabstyleRight(void) const { return m_nbTabStyle == wxRIGHT; }
	wxDockingInfo &tabstyleTop(void) { m_nbTabStyle = wxTOP; return *this; }
	bool isTabstyleTop(void) const { return m_nbTabStyle == wxTOP; }
	wxDockingInfo &tabstyleBottom(void) { m_nbTabStyle = wxBOTTOM; return *this; }
	bool isTabstyleBottom(void) const { return m_nbTabStyle == wxBOTTOM; }
	long tabStyle(void) const;

	// Toolbars
	wxDockingInfo &toolbarTop(void) { m_horizontal = true; m_direction = wxTOP; return *this; }
	bool isToolbarTop(void) const { return m_horizontal == true && m_direction == wxTOP; }
	wxDockingInfo &toolbarBottom(void) { m_horizontal = true; m_direction = wxBOTTOM; return *this; }
	bool isToolbarBottom(void) const { return m_horizontal == true && m_direction == wxBOTTOM; }

	wxDockingInfo &toolbarLeft(void) { m_horizontal = false; m_direction = wxLEFT; return *this; }
	bool isToolbarLeft(void) const { return m_horizontal == false && m_direction == wxLEFT; }
	wxDockingInfo &toolbarRight(void) { m_horizontal = false; m_direction = wxRIGHT; return *this; }
	bool isToolbarRight(void) const { return m_horizontal == false && m_direction == wxRIGHT; }

	bool isToolbarHorizontal(void) const { return m_horizontal; }
	bool isToolbarVertical(void) const { return !m_horizontal; }

	// Toolbar/Floating
	wxDockingInfo &position(wxPoint &point) { m_point = point; return *this; }
	wxPoint position(void) const { return m_point; }

	wxDockingInfo &size(wxSize &size) { m_size = size; return *this; }
	wxSize size(void) const { return m_size; }

private:
	static wxDockingInfo *m_default;

	wxDockingPanel *m_dockingPanel;
	wxString m_title;

	// Splitting
	wxDirection m_direction;

	wxDirection m_nbTabStyle;

	// Toolbars
	bool m_horizontal:1;

	// Toolbar/Floating
	wxPoint m_point;
	wxSize m_size;

	bool m_activate;
};

#endif // wxUSE_DOCKING
