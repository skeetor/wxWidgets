#pragma once

#include "wx/defs.h"

#if wxUSE_DOCKING
#include <vector>

#include <wx/frame.h>

#include <wx/docking/docking_defs.h>
#include <wx/docking/dockinginfo.h>

class wxDockingPanel;
class wxGridBagSizer;

/**
 * wxDockingFrame provides the main frame window handling docking
 * to it's layout.
 * Direction must always be one of wxNONE(center), wxLEFT, wxRIGHT, wxUP, wxDOWN.
 */
class WXDLLIMPEXP_DOCKING wxDockingFrame
: public wxFrame
{
public:
	wxDockingFrame();

	wxDockingFrame(wxWindow *parent,
		wxWindowID id,
		const wxString &title,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString &name = wxASCII_STR(wxFrameNameStr));

	wxDockingFrame(wxWindow *parent,
		wxWindowID id,
		wxDockingPanel *root,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString &name = wxASCII_STR(wxFrameNameStr));

	~wxDockingFrame() override;

	bool Create(wxWindow *parent,
		wxWindowID id,
		const wxString &title,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString &name = wxASCII_STR(wxFrameNameStr));

	wxDockingInfo &Defaults(void) { return m_defaults; }
	wxDockingInfo const &Defaults(void) const { return m_defaults; }

	/**
	 * Find the wxDockingPanel where the specified window resides in. If the window
	 * is already a wxDockingPanel, then this is returned.
	 */
	wxDockingPanel *FindDockingParent(wxWindow *window) const;

	/**
	 * Return the tab panel associated with this window.
	 * If no tab panel is found, a nullpointer is returned. If the window itself is a
	 * tab panel, then this is returned.
	 */
	wxDockingPanel *FindTabParent(wxWindow *window) const;

	/**
	 * Find the tab parent for the current window, only if it is a direct descendant
	 * of a tab panel.
	 */
	wxDockingPanel *FindDirectTabParent(wxWindow *window) const;

	/**
	 * The rootPanel holds the panel, which is directly parented to the wxDockingFrame.
	 * This may change, depending on the layout requirements, so this pointer may not be
	 * held after the layout has been modified. This pointer will never be null.
	 * After a call to any of the layout functions like AddPanel, TabifyPanel, etc. it is not
	 * guaranteed that this pointer is still valid, or that it still points to the same content.
	 */
	wxDockingPanel *GetRootPanel(void) const { return m_rootPanel; }

	/**
	 * Returns the currently active panel. This might be a nullptr if no panel is active.
	 */
	wxDockingPanel *GetActivePanel(void) const { return m_activePanel; }

	/**
	 * Add the panel relative to the specified panel in the given direction.
	 * If the dockinpPanel is a nullptr, it will be docked to the respective border.
	 * 
	 * @param panel The user provided panel. This window will be reparented
	 * to the layout, so no assumpetions should be done about a particular
	 * window hierarchy.
	 * @param title A title string, which will be used when the window is
	 * floating or turned into a tab. It might also be used for a drag handle
	 * if applicable.
	 * 
	 * If successfull, the dockingpanel which it is attached to is returned,
	 * otherwise a nullptr.
	 */
	wxDockingPanel *AddPanel(wxWindow *panel, wxDockingInfo const &info, wxDockingPanel **notebook = nullptr);

	/**
	 * Add the panel to a tab in the dockiongPanel. If the dockingPanel doesn't
	 * have tabs, it will be converted accordingly.
	 * If the notebook pointer is provided, the notebook will be returned as
	 * well. If showTab() is not set in the info and it is the first panel,
	 * then no tab is created and this pointer will be null.
	 */
	wxDockingPanel *AddTabPanel(wxWindow *panel, wxDockingInfo const &info, wxDockingPanel **notebook = nullptr);

	/**
	 * Split the dockingPanel and add the panel in the specified direction.
	 */
	wxDockingPanel *SplitPanel(wxWindow *panel, wxDockingInfo const &info);

	/**
	 * Create a floating dockingPanel and insert the panel.
	 */
	wxDockingPanel *FloatPanel(wxWindow *panel, wxDockingInfo const &info);

	/**
	 * Adds a new toolbar to the frame.
	 */
	wxDockingPanel *AddToolBar(wxToolBar *toolbar, wxDockingInfo const &info);

	/**
	 * HideToolbar will hide the toolbar, so it is no longer visible. The toolbar
	 * is still associated to the frame, and can be shown again. It may not be
	 * reattached by AddToolBar().
	 */
	wxDockingPanel *HideToolBar(wxToolBar *toolbar, wxDockingInfo const &info);

	/**
	 * ReoveToolbar will detach the toolbar from the frame. After this, the toolbar
	 * is no longer associated with a window, and the user is responsible for deleting
	 * the toolbar. After this the toolbar has to be reattached by using AddToolBar()
	 * if desired.
	 */
	bool RemoveToolBar(wxToolBar *toolbar, wxDockingInfo const &info);

	void SetActivePanel(wxDockingPanel *panel);

	/**
	 * Serialize the current layout to a string, which allows to restore this layout later.
	 * This string is also suitable for persisting the layout into a config file to be able
	 * to restore the layout after the application restarts.
	 */
	//wxString SerializeLayout(void) const;

	/**
	 * Only serialize this frame.
	 */
	//wxString SerializeFrame(void) const;

	/**
	 * Deserialize the layout from a string created by serializeLayout(). It should be noted
	 * that the application is responsible for creating the appropriate panels, as this function
	 * can only restore the layout itself, but not the panels created by the application.
	 */
	//bool DeserializeLayout(wxString layout);

	/**
	 * Only deserialize this frame.
	 */
	//bool DeserializeFrame(wxString layout);

protected:
	void OnMouseLeftDown(wxMouseEvent &event);

	void UpdateToolbarLayout(void);
	bool HideToolbar(wxDockingPanel *&toolbar);

private:
	void init(void);
	void BindEventHandlers(void);
	void UnbindEventHandlers(void);

private:
	wxDockingPanel *m_rootPanel;
	wxDockingPanel *m_activePanel;

	// Some values are used as defaults if not specified in a function.
	wxDockingInfo m_defaults;

	// Toolbar members
	wxGridBagSizer *m_sizer;
	wxDockingPanel *m_toolbarsLeft;
	wxDockingPanel *m_toolbarsRight;
	wxDockingPanel *m_toolbarsTop;
	wxDockingPanel *m_toolbarsBottom;

	wxDECLARE_DYNAMIC_CLASS(wxDockingFrame);
};

#endif // wxUSE_DOCKING
