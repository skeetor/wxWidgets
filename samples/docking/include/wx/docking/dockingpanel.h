#pragma once

#include "wx/defs.h"

#if wxUSE_DOCKING

#include <wx/panel.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include <wx/toolbar.h>
#include <wx/docking/dockingframe.h>

#include <wx/docking/docking_defs.h>

class wxDockingFrame;
/**
 * wxDockingPanel represents a single view in the dockable window framework.
 * It can have the following states:
 *    - Floating: It shows at least one panel in a seperate window
 *    - Tabbed: It shows one or more panels
 *    - Splitted: It shows two panels.
 *    - User: This view represents a user panel
 * 
 * Any single instance of wxDockingPanel can only be in one state.
 */
class WXDLLIMPEXP_DOCKING wxDockingPanel
: public wxPanel
{
	friend wxDockingFrame;

public:
	wxDockingPanel();

	wxDockingPanel(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		wxString const &title = "",
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxTAB_TRAVERSAL | wxNO_BORDER,
		const wxString &name = wxASCII_STR(wxPanelNameStr));

	~wxDockingPanel() override;

	wxString const &GetTitle(void) const { return m_title; }
	void SetTitle(wxString const &title) { m_title = title; }

	bool isFloating(void) const { return m_floating; }
	bool isSplitted(void) const { return m_splitted; }
	bool isTabbed(void) const { return m_tabbed; }
	bool isUser(void) const { return m_window; }

protected:
	void OnSize(wxSizeEvent &event);

protected: // API
	wxWindow *GetUserPanel(void) const { return (m_window) ? m_panel : nullptr; }
	void SetUserPanel(wxWindow *panel)
	{
		m_panel = panel;
		initFlags();
		m_window = true;
	}

	wxSplitterWindow *GetSplitter(void) const { return (m_splitted) ? m_splitter : nullptr; }
	void SetSplitter(wxSplitterWindow *splitter)
	{
		m_splitter = splitter;
		initFlags();
		m_splitted = true;
	}

	wxNotebook *GetNotebook(void) const { return (m_tabbed) ? m_notebook : nullptr; }
	void SetNotebook(wxNotebook *notebook)
	{
		m_notebook = notebook;
		initFlags();
		m_tabbed = true;
	}

	wxDockingFrame *GetFloatingWindow(void) const { return (m_floating) ? m_frame : nullptr; }
	void SetFloatingWindow(wxDockingFrame *panel)
	{
		m_frame = panel;
		initFlags();
		m_floating = true;
	}

	wxToolBar *GetToolbar(void) const { return (m_toolbar) ? m_toolbarWindow : nullptr; }
	void SetToolbar(wxToolBar *toolbar)
	{
		m_toolbarWindow = toolbar;
		initFlags();
		m_toolbar = true;
	}

private:
	void init(wxString const &title = "");
	void initFlags(void)
	{
		m_window = false;
		m_floating = false;
		m_splitted = false;
		m_tabbed = false;
		m_toolbar = false;
	}

private:
	union
	{
		wxWindow *m_panel;
		wxSplitterWindow *m_splitter;
		wxNotebook *m_notebook;
		wxDockingFrame *m_frame;
		wxToolBar *m_toolbarWindow;
	};

	wxString m_title;
	bool m_window : 1;
	bool m_floating : 1;
	bool m_splitted : 1;
	bool m_tabbed : 1;
	bool m_toolbar : 1;

	wxDECLARE_EVENT_TABLE();
	// We don't allow copying because we need to keep track of the panels.
	wxDECLARE_DYNAMIC_CLASS_NO_COPY(wxDockingPanel);
};

#endif // wxUSE_DOCKING
