#ifndef _WX_DOCKING_INFO_H_
#define _WX_DOCKING_INFO_H_

#include <wx/defs.h>

#if wxUSE_DOCKING

#include <wx/string.h>
#include <wx/gdicmn.h>
#include <wx/docking/docking_defs.h>
#include <wx/docking/dockingentity.h>
#include <wx/docking/dockingutils.h>

class WXDLLIMPEXP_DOCKING wxDockingInfo
{
public:
	wxDockingInfo(wxString const &title = "");
	~wxDockingInfo()
	{
	}

	void Clear();

	/**
	 * This function will update all fields relevant for the given window which
	 * is currently docked.
	 * The specified window can be any child window residing inside the docking
	 * system (i.E. a button on a panel). If the window is not part of the
	 * docking system, false is returned.
	 */
	bool FromWindow(wxWindow *source, wxDockingFrame *frame);

	/**
	 * This function will update all fields relevant for the given window which
	 * is found at the (screen) coordinates.
	 * If no window is found, then false is returned.
	 */
	bool FromPoint(wxPoint coordinates, wxDockingFrame *frame, wxWindow **pointWindow = nullptr)
	{
			wxWindow *w = wxFindWindowAtPoint(coordinates);

			// We report the actual found window, as it may be reported differently after evaluation is done.
			if (pointWindow)
				*pointWindow = w;

			return FromWindow(w, frame);
	}

	/**
	 * Set the default values for all instances.
	 */
	static wxDockingInfo &GetDefaults();

	/**
	 * Returns the panel which should be considered as the docking panel. This is normaly the window if that is
	 * null then it uses the panel and if that is also null then the frame.
	 */
	wxDockingEntity GetDockingEntity() const;

	wxDockingInfo &SetFrame(wxDockingFrame *frame) { m_frame = frame; return *this; }
	wxDockingFrame *GetFrame() const { return m_frame; }

	wxDockingInfo &SetPanel(wxDockingEntity const &panel) { m_panel = panel; return *this; }
	wxDockingInfo &SetPanel(wxWindow *panel, wxDockingEntityType type) { m_panel = wxDockingEntity(panel, type); return *this; }
	wxDockingEntity &GetPanel() { return m_panel; }
	wxDockingEntity const &GetPanel() const { return m_panel; }

	wxDockingInfo &SetWindow(wxDockingEntity const &panel) { m_window = panel; return *this; }
	wxDockingInfo &SetWindow(wxWindow *window, wxDockingEntityType type) { m_window = wxDockingEntity(window, type); return *this; }
    wxDockingEntity &GetWindow() { return m_window; }
    wxDockingEntity const &GetWindow() const { return m_window; }

	wxDockingInfo &SetTitle(wxString const &title) { m_title = title; return *this; }
	wxString const &GetTitle() const { return m_title; }

	wxDockingInfo &SetDirection(wxDirection direction) { m_direction = direction; return *this; }
	wxDirection GetDirection() const { return m_direction; }

	// Notebook
	wxDockingInfo &SetTabDirection(wxDirection direction) { m_tabDirection = direction; return *this; }
	wxDirection GetTabDirection() const { return m_tabDirection; }

	wxDockingInfo &SetTabArea(bool flag) { m_tabArea = flag; return *this; }
	bool IsTabArea() const { return m_tabArea; }

	wxDockingInfo &SetOnTab(bool flag) { m_onTab = flag; return *this; }
	bool IsOnTab() const { return m_onTab; }

	long GetTabStyle() const;

	bool isActive() const { return m_activate; }
	void SetActivate(bool activate = true) { m_activate = activate; }

	bool isFloating() const { return m_floating; }
	void SetFloating(bool floating = true) { m_floating = floating; }

	/**
	 * ForcePanel will create a tab or a splitter in thos cases where it is not strictly needed. For example, if the frame is empty
	 * on startup and you add the first user window to it, it will simply be put into the frame by default. If you set the forcePanel
	 * flag, then it will still create a tab as well for it (or a splitter, depending on the direction setting like CENTER or LEFT/RIGHT etc,).
	 */
	bool IsForcePanel() const { return m_forcePanel; }
	wxDockingInfo &SetForcePanel(bool force) { m_forcePanel = force; return *this; }

	/**
	 * When a window is splitted and there is no second window, this option will still split the window and put a placeholder in the empty
	 * area. Default is to create the splitter in unsplitted mode, so it will look like a regular window.
	 */
	bool IsForceSplit() const { return m_forceSplit; }
	wxDockingInfo &SetForceSplit(bool force) { m_forceSplit = force; return *this; }

    size_t GetPage() const { return m_page; }
	void SetPage(size_t page) { m_page = page; }

	wxDockingInfo &SetOrientation(wxOrientation orientation);
	wxOrientation GetOrientation() const { return m_orientation; }

	// Toolbar/Floating
	wxDockingInfo SetRect(wxRect const &rect) { SetPosition(rect.x, rect.y); SetSize(rect.width, rect.height); return *this; }
	wxRect GetRect() const { return wxRect(m_position.x, m_position.y, m_size.x, m_size.y); }

	wxDockingInfo &SetPosition(wxPoint const &position) { m_position = position; return *this; }
	wxDockingInfo &SetPosition(int x, int y) { m_position = wxPoint(x, y); return *this; }
	wxPoint GetPosition() const { return m_position; }

	wxDockingInfo &SetSize(int width, int height) { m_size = wxSize(width, height); return *this; }
	wxDockingInfo &SetSize(wxSize const &size) { m_size = size; return *this; }
	wxSize GetSize() const { return m_size; }

private:
	wxDockingFrame *m_frame;
	wxDockingEntity m_panel;
    wxDockingEntity m_window;
	wxString m_title;
	wxDirection m_direction;
	wxOrientation m_orientation;

	wxPoint m_position;
	wxSize m_size;

	wxDirection m_tabDirection;
    size_t m_page;					// Index of the page, if the panel is a notebook

	bool m_activate:1;
	bool m_floating:1;
	bool m_forcePanel : 1;			// Create an appropritate panel even for the initial window. Default is none.
	bool m_forceSplit : 1;			// Split the windows even when there is no second window yet.
	bool m_tabArea : 1;				// Is set to true if the mouse is within the tab area.
									// This does not neccessarily mean that the mouse is over a tab. It can also be in the fre area when there are not enough tabs.
	bool m_onTab : 1;				// Is set to true if the mouse is over a tab.
};

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_INFO_H_
