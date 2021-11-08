#pragma once

#include "wx/defs.h"

#if wxUSE_DOCKING
#include <vector>

#include <wx/frame.h>

#include <wx/docking/docking_defs.h>
#include <wx/docking/dockinginfo.h>

class wxDockingPanel;
class wxGridBagSizer;
class wxNotebook;

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
	 * is already a wxDockingPanel, then this is returned. The docking parent is not
	 * neccessarily the direct parent of this window.
	 */
	wxDockingPanel *FindDockingParent(wxWindow *window) const;

	/**
	 * Return the dockingpanel if the user window is directly associated with it.
	 */
	wxDockingPanel *FindDirectParent(wxWindow *window) const;

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
	void SetActivePanel(wxDockingPanel *panel);

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

	/**
	 * Remove the specified window from the docking. If the panel is part of a notebook
	 * the page will be removed. If it was the last page, the notebook will also be deleted.
	 * If the serwindow is part of a splitter, or a notebook inside a splitter, the split
	 * window is unsplitted and also removed.
	 *
	 * Note: The client should never hold on to a dockingpanel, splitter or notebook as those
	 * might be deleted in the process and are handled internally.
	 */
	void Undock(wxWindow *userWindow);

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

	void OnSize(wxSizeEvent &event);

protected:
	void DoSize(void);

	/**
	 * Create a wxNotebook tab panel with the userWindow as it's page. If the userWindow is
	 * a wxNotebook it will not create a new one, instead it adds the userWindow to it. If the
	 * parent is a nullpointer the parent of the user window will be used as the parent for the
	 * wxDockingTarget. If the info doesn't contain a docking target, a new wxDockingPanel will
	 * be created.
	 */
	wxDockingPanel *CreateTabPanel(wxWindow *userWindow, wxDockingInfo const &info, wxWindow*parent = nullptr);

	/**
	 * Remove the panel from the docking. The panel is not destroyed itself, even though the docked
	 * panel can be destroyed if it becomes empty. The panel can still be docked to some other
	 * target.
	 */
	bool RemovePanel(wxDockingPanel *panel);

	void OnMouseLeftDown(wxMouseEvent &event);

	void UpdateToolbarLayout(void);
	bool HideToolbar(wxDockingPanel *&toolbar);

	wxNotebook *ConvertToNotebook(wxWindow *source, wxWindow *pageWindow, wxDockingInfo const &info);

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
