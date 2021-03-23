#ifndef _WX_DOCKINGFRAME_H_
#define _WX_DOCKINGFRAME_H_

#include "wx/defs.h"

#if wxUSE_DOCKING
#include <vector>

#include <wx/frame.h>
#include <wx/docking/docking_defs.h>

class wxDockingPanel;

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
	~wxDockingFrame() override;

	bool Create(wxWindow *parent,
		wxWindowID id,
		const wxString &title,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString &name = wxASCII_STR(wxFrameNameStr));

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
	 * If successfull, the dockingpanel which it is attached to, is returned,
	 * otherwise a nullptr.
	 */
	wxDockingPanel *AddPanel(wxWindow *panel, wxString const &title, wxDirection direction, wxDockingPanel *dockingPanel = nullptr);
	wxDockingPanel *TabifyPanel(wxWindow *panel, wxString const &title, wxDockingPanel *dockingPanel);
	wxDockingPanel *SplitPanel(wxWindow *panel, wxString const &title, wxDirection direction, wxDockingPanel *dockingPanel);

protected:
	void OnSize(wxSizeEvent &event);
	void OnChildFocus(wxChildFocusEvent &event);

	wxDockingPanel *getRootPanel(void) const { return m_rootPanel;  }

private:
	/**
	 * The rootPanel holds the panel, which is directly parented to the wxDockingFrame.
	 * This may change, depending on the layout requirements, so this pointer may not be
	 * held by outside code. This pointer will never be null.
	 */
	wxDockingPanel *m_rootPanel;
	wxDockingPanel *m_activePanel;

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_DYNAMIC_CLASS(wxDockingFrame);
};

#endif // wxUSE_DOCKING

#endif // _WX_DOCKINGFRAME_H_
