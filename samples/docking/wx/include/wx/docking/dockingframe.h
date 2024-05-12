#ifndef _WX_DOCKING_FRAME_H_
#define _WX_DOCKING_FRAME_H_

#if wxUSE_DOCKING

#include "wx/defs.h"
#include <wx/gbsizer.h>
#include <wx/frame.h>
#include <wx/splitter.h>
#include <wx/notebook.h>

#include <wx/docking/docking_defs.h>
#include <wx/docking/dockingevent.h>
#include <wx/docking/dockinginfo.h>
#include <wx/docking/dockingstate.h>
#include <wx/docking/dockingoverlay.h>

/**
 * wxDockingFrame provides the main frame window handling docking
 * to it's layout.
 * Direction must always be one of wxNONE(center), wxLEFT, wxRIGHT, wxUP, wxDOWN.
 */
class WXDLLIMPEXP_DOCKING wxDockingFrame
: public wxFrame
{
	friend class wxDockingEventFilter;

public:
	wxDockingFrame();

	wxDockingFrame(wxWindow *parent,
		wxWindowID id,
		const wxString &title,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString &name = wxASCII_STR(wxFrameNameStr));

	~wxDockingFrame() wxOVERRIDE;

	bool Create(wxWindow *parent,
		wxWindowID id,
		const wxString &title,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString &name = wxASCII_STR(wxFrameNameStr));

public: // API
	wxDockingEntity AddPanel(wxDockingInfo const &info, wxDockingEntity const &userWindow);

	/**
	 * Remove the specified window from the docking. If the panel is part of a notebook
	 * the page will be removed. If the page is the last page, the notebook may be destroyed
	 * and the window converted to a child window of the notebooks parent. This can be vetoed
	 * if desired.
	 * If it was the last page, the notebook will also be deleted which also may be vetoed to keep
	 * the notebook around.
	 * The window provided by the user, will always be reparented to the wxDockingFrame. So if it
	 * is to be reused, it might need to be reparented appropriatly. If it is to be docked again
	 * somewhere else, then this is not needed, as the docking module will do this internally to
	 * the correct target panel.
	 * If allowParentDestroy is false, then the parent container will not be destroyed if it is
	 * elligible for it. If the parent is locked, it will not be destroyed anyway.
	 *
	 * RETURN: true if sucessful.
	 */
	bool RemovePanel(wxDockingEntity &panel, bool allowParentDestroy = true);

	/**
	 * Moves a window, which already has to exist in the docking system, to
	 * a new destination. If the window does not exist in the docking system
	 * false is returned.
	 */
	bool MovePanel(const wxDockingInfo &source, const wxDockingInfo &target);

	/**
	 * Replace the old window with the new one. The old window can be reused or deleted after that. If the parent
	 * is a wxNotebook the title will be used for the tab. In case of a frame it will set the frame title.
	 */
	bool ReplaceWindow(wxDockingEntity const &oldWindow, wxDockingEntity const &newWindow, wxString const &title, wxDockingEntity parent);

	/**
	 * Serialize the current layout to a string, which allows to restore this layout later.
	 * This string is also suitable for persisting the layout into a config file to be able
	 * to restore the layout after the application restarts.
	 */
	//wxString SerializeLayout() const;

	/**
	 * Deserialize the layout from a string created by serializeLayout(). It should be noted
	 * that the application is responsible for creating the appropriate panels, as this function
	 * can only restore the layout itself, but not the panels created by the application.
	 */
	//bool DeserializeLayout(wxString layout);

	/**
	 * Only serialize this frame.
	 */
	//wxString SerializeFrame() const;

	/**
	 * Only deserialize this frame.
	 */
	//bool DeserializeFrame(wxString layout);

public:
	void OnSize(wxSizeEvent &event);

protected: // Tracking
	void BeginTracking(wxDockingEvent &event);
	void EndTracking(wxDockingEvent &event);

	/**
	 * Initializes the target and updates LastKnownTarget field in the event
	 * if appropriate. The field is untouched if the criterias are not met.
	 */
	void InitTarget(wxDockingEvent &event);

	/**
	 * Restores the window content  if it has changed. Returns true if the window was refreshed.
	 */
	bool RestoreWindow(wxDockingEntity &window, wxDirection newDirection, bool allow);
	void PaintFilterOverlay(wxRect &rect, bool allowed, wxDockingEntity &panel, wxColor allowedColor);

	void ShowGhostFrame(wxPoint pos, wxSize size);
	bool UpdateWindowHint(wxDockingEntity &srcWindow, wxDockingEntity &targetWindow, wxPoint pos, wxDirection direction, bool allow);
	void UpdateTabHint(wxDockingInfo &target, wxPoint const &mousePos, bool allow);

protected: // Events
	/**
	 * Sent when the Docking is about to be started. The source field is populated where the
	 * docking starts from. If this event is vetoed, then no further docking events will be generated
	 * and docking is disabled until the next time the mouse is used to trigger a docking again.
	 */
	bool SendDockingStart();
	void SendDockingEnd();

	void SendCreatePanel(wxDockingInfo &info, wxDockingEntityType type);
	void SendReleasePanel(wxDockingEntity const &info);
	void SendMovePanel(wxDockingEvent &event);
	void SendTrackMove(wxDockingEvent &event);
	void SendUpdateHint(wxDockingEvent &event);

	void OnDockingStart(wxDockingEvent &event);
	void OnDockingEnd(wxDockingEvent &event);
	void OnTrackMove(wxDockingEvent &event);
	void OnUpdateHint(wxDockingEvent &event);

	/**
	 * When the user drags around his windows, a panel can become empty, in which case it will be no longer needed.
	 * In this case we ask the client before we remove such a panel as well. The client may veto this if the panel should
	 * persist.
	 */
	bool SendTryRemovePanel(wxDockingInfo const &info) const;

	/**
	 * The target panel type must be set to the desired docking panel type. The panel pointer
	 * should not contain a valid pointer as it will be overwritten, so the pointer is lost.
	 * The target Window member must be set to the parent of the new panel.
	 * Depending on the type additional values can be set in the target.
	 *
	 * wxDOCKING_NOTEBOOK: direction for the tabs.
	 */
	void OnCreateDockingPanel(wxDockingEvent &event);

	/**
	 * The target panel pointer is no longer part of the docking system and can be deleted
	 * or reused for another docking operation.
	 */
	void OnReleaseDockingPanel(wxDockingEvent &event);

	void OnMovePanel(wxDockingEvent &event);

protected: // Helpers
	/**
	 * Set the specified window in the frame. Depending on the direciton setting a required
	 * tab or splitter is created. The return panel is the one where the user panel was attached to.
	 * If the frame was empty, the window is simply set into it. If the force flag is set, then the tab
	 * or splitter is also created in this case.
	 */
	wxDockingEntity AddFramePanel(wxDockingInfo const &info, wxDockingEntity const &panel);

	/**
	 * Create a floating dockingPanel and insert the panel.
	 */
	wxDockingEntity AddFloatPanel(wxDockingInfo const &info, wxDockingEntity userWindow);

	/**
	 * Add the panel to a tab in the dockinggPanel. If the dockingPanel doesn't
	 * have tabs, it will be converted accordingly.
	 */
	wxDockingEntity AddTab(wxDockingInfo const &info, wxDockingEntity const &panel);

	/**
	 * Creates a new wxNotebook tab panel with the userWindow as it's page. If the userWindow is nullptr then an empty
	 * wxNotebook is created, otherwise the page is either added or inserted at the specified index.
	 * If the parent is nullptr then the notebook is parented to the frame from the info object.
	 * If the frame is not set, then the current frame will be used.
	 */
	wxDockingEntity CreateTabPanel(wxDockingInfo const &info, wxDockingEntity const &userWindow = nullptr, wxDockingEntity parent = nullptr);

	bool AttachTabPage(wxDockingInfo const &info, wxDockingEntity const &notebook, wxDockingEntity const &userWindow) const;

	/**
	 * Adds a new toolbar to the frame.
	 */
	wxDockingEntity AddToolBar(wxToolBar *toolbar, wxDockingInfo const &info);

	/**
	 * HideToolbar will hide the toolbar, so it is no longer visible. The toolbar
	 * is still associated to the frame, and can be shown again. It may not be
	 * reattached by AddToolBar().
	 */
	void HideToolBar(wxToolBar *toolbar, wxDockingInfo const &info);

	/**
	 * RemoveToolbar will detach the toolbar from the frame. After this, the toolbar
	 * is no longer associated with a window, and the user is responsible for deleting
	 * the toolbar. After this the toolbar has to be reattached by using AddToolBar()
	 * if desired.
	 */
	bool RemoveToolBar(wxToolBar *toolbar, wxDockingInfo const &info);

	/**
	 * Helper function to create a panel of the specified type if no extra parameters are required
	 */
	wxDockingEntity CreatePanel(wxDockingEntity &parent, wxDockingEntityType type)
	{
		wxDockingInfo info;
		info.SetWindow(parent);
		SendCreatePanel(info, type);
		return info.GetWindow();
	}

	void DoSize();

	/**
	 * Move panel only when source and target are both in the same splitter.
	 */
	bool DoMoveSplitter(wxDockingInfo const &src, wxDockingInfo const &tgt);

	/**
	 * Remove the notebook or a page from it, as specified in wxDockingInfo.
	 * If the last page of a notebook is removed it might be destroyed. If the notebook is
	 * locked, it will stay, otherwise an event is sent where the client can decide to keep
	 * or destroy it.
	 * If allowDestroy is false, then this check will not be performed and no event is sent as
	 * the notebook will stay, thus overriding those checks.
	 * The removed panel is returned. At this stage, the panel is not destroyed and the caller
	 * is responsible for the lifetime of it. The returned window can be safely destroyed.
	 */
	bool RemoveTab(wxDockingEntity &notebook, wxDockingEntity &page, bool allowDestroy);
	bool RemoveSplitter(wxDockingEntity &splitter, wxDockingEntity &panel, bool allowDestroy);
	bool RemoveFrame(wxDockingEntity &frame, wxDockingEntity &panel, bool allowDestroy);

	/**
	 * Remove the window from the panel and reattaches it to the otherPanel. The source panel may be destroyed
	 * if appropriate.
	 */
	bool MoveToPanel(wxDockingEntity const &otherPanel, wxDockingEntity const &panel, wxWindow *window);

	void SetRootPanel(wxDockingEntity const &root) { m_rootPanel = root; }
	wxDockingEntity const &GetRootPanel() const { return m_rootPanel; }
	wxDockingEntity GetRootPanel() { return m_rootPanel; }

	/**
	 * Split the dockingPanel and add the panel in the specified direction.
	 */
	wxDockingEntity AddSplitter(wxDockingInfo const &info, wxDockingEntity userWindow);

	/**
	 * Create a new wxSplitterWindow panel. 
	 */
	wxDockingEntity CreateSplitPanel(wxDockingInfo const &info, bool doSplit, wxDockingEntity firstWindow = nullptr, wxDockingEntity secondWindow = nullptr, wxDockingEntity parent = nullptr);

	/**
	 * Attach the windows into the splitter
	 */
	bool AttachSplitPanel(wxDockingInfo const &info, wxSplitterWindow *splitter, wxDockingEntity firstWindow, wxDockingEntity secondWindow);

	int OnMouseLeftDown(wxMouseEvent &event);
	int OnMouseLeftUp(wxMouseEvent &event);
	int OnMouseMove(wxMouseEvent &event);
	void OnMouseCaptureLost(wxMouseCaptureLostEvent &event);

	/**
	 * Stops the current docking session. If affirm == true, then a move is triggered otherwise
	 * the current docking is simply canceled.
	 */
	void EndDocking(wxPoint const &mousePos, bool affirm);

	void OnSplitterSashUpdate(wxSplitterEvent &event);
	void OnSplitterDClick(wxSplitterEvent &event);

	void UpdateToolbarLayout();
	//bool DoHideToolbar(wxToolBar *toolbar);

	/**
	 * Replace a page from a notebook with a new notebook containing the original page.
	 * Returns the page and the index if successfull, or nullptr if the page was not found.
	 */
	wxNotebook *ReplaceNotebookPage(wxNotebook *notebook, wxDockingEntity const &oldPage, int &index, wxDockingInfo const &info);

	/**
	 * Set the size of the bar that indicates the docking target.
	 * This is also threshold where the mouse is allowed to start a docking dragging operation.
	 * i.E. If set to 20 the mouse can be up to 20 pixels away from the top border to start dragging.
	 */
	void setDockingWidth(uint32_t width) { m_dockingWidth = width; }
	uint32_t getDockingWidth() const { return m_dockingWidth; }

	void setDockingTabWidth(uint32_t width) { m_dockingTabWidth = width; }
	uint32_t getDockingTabWidth() const { return m_dockingTabWidth; }

	/**
	 * Validate if the source window can be moved to the target window. This can not happen
	 * if the target resides inside the source. While a notebook can move the tabs around, it
	 * still has the same limitation, that a window can not be moved into a child of itself.
	 */
	bool CheckChildState(wxDockingInfo const &src, wxDockingInfo const &tgt);

	/**
	 * Check if the docking conditions are met, so we have an actual docking drag case. Coordinates are
	 * in screen coordinates.
	 */
	bool DockingStartCondition(wxPoint const &mousePos) const;

	bool UpdateDirection(wxDockingInfo const &src, wxDockingInfo &tgt, wxPoint const &mousePos);

private:
	void init();
	void BindEventHandlers();
	void UnbindEventHandlers();
	void RemoveFrameEntry() const;

private:
	wxDockingEntity m_rootPanel;

	// Some values are used as defaults if not specified in a function.
	wxDockingInfo m_defaults;

	// Toolbar members
	wxGridBagSizer *m_sizer;
	wxDockingEntity m_toolbarsLeft;
	wxDockingEntity m_toolbarsRight;
	wxDockingEntity m_toolbarsTop;
	wxDockingEntity m_toolbarsBottom;

	uint32_t m_dockingWidth;
	uint32_t m_dockingTabWidth;

private:	// Tracking specific variables
	// The panel which shows the current GUI hint.
	wxDockingEntity m_curPanel;
	wxDirection m_curDirection;
	bool m_curAllow:1;
	wxPoint m_mousePos;

	// TODO: Not sure if these need to be accessible from different frames and thus be static. These are needed for showing a ghostframe when floating.
	wxBitmap m_sourceWindow;
	std::unique_ptr<wxFrame> m_screenWindow;

private: // wxWidget
	wxDECLARE_DYNAMIC_CLASS(wxDockingFrame);
	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(wxDockingFrame);
};

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_FRAME_H_
