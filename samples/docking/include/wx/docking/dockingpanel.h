#pragma once

#include "wx/defs.h"

#if wxUSE_DOCKING

#include <wx/panel.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include <wx/toolbar.h>
#include <wx/docking/dockingframe.h>

#include <wx/docking/docking_defs.h>

enum wxDockingPanelType
{
	wxDOCKING_NONE,
	wxDOCKING_USER,
	wxDOCKING_PANEL,
	wxDOCKING_FLOATING,
	wxDOCKING_SPLITTED,
	wxDOCKING_TABBED,
	wxDOCKING_TOOLBAR

};

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
		wxString const &title = "",
		wxWindowID id = wxID_ANY,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = wxTAB_TRAVERSAL | wxNO_BORDER,
		const wxString &name = wxASCII_STR(wxPanelNameStr));

	~wxDockingPanel() override;

	wxString const &GetTitle(void) const { return m_title; }
	void SetTitle(wxString const &title) { m_title = title; }

	bool empty(void) const { return m_userWindow == nullptr; }
	bool isFloating(void) const { return m_type == wxDOCKING_FLOATING; }
	bool isSplitted(void) const { return m_type == wxDOCKING_SPLITTED; }
	bool isTabbed(void) const { return m_type == wxDOCKING_TABBED; }
	bool isUser(void) const { return m_type == wxDOCKING_USER; }
	bool isToolBar(void) const { return m_type == wxDOCKING_TOOLBAR; }
	bool isPanel(void) const { return m_type == wxDOCKING_PANEL; }

	wxWindow *GetWindow(void) const { return m_userWindow; }

	wxDockingPanel *GetDockingPanel(void) const { return (isPanel()) ? m_dockingPanel : nullptr; }
	wxWindow *GetUserWindow(void) const { return (isUser()) ? m_userWindow : nullptr; }
	wxSplitterWindow *GetSplitter(void) const { return (isSplitted()) ? m_splitter : nullptr; }
	wxNotebook *GetNotebook(void) const { return (isTabbed()) ? m_notebook : nullptr; }
	wxDockingFrame *GetFloatingWindow(void) const { return (isFloating()) ? m_frame : nullptr; }
	wxToolBar *GetToolbar(void) const { return (isToolBar()) ? m_toolbarWindow : nullptr; }

	void TakeDocking(wxDockingPanel const &source);
	void TakeDocking(wxDockingPanel const *source)
	{
		TakeDocking(*source);
	}

protected:
	void OnSize(wxSizeEvent &event);

protected: // API

	void SetDockingPanel(wxDockingPanel *panel)
	{
		m_dockingPanel = panel;
		m_type = wxDOCKING_PANEL;
	}

	void SetUserWindow(wxWindow *panel)
	{
		m_userWindow = panel;
		m_type = wxDOCKING_USER;
	}

	void SetSplitter(wxSplitterWindow *splitter)
	{
		m_splitter = splitter;
		m_type = wxDOCKING_SPLITTED;
	}

	void SetNotebook(wxNotebook *notebook)
	{
		m_notebook = notebook;
		m_type = wxDOCKING_TABBED;
	}

	void SetFloatingWindow(wxDockingFrame *panel)
	{
		m_frame = panel;
		m_type = wxDOCKING_FLOATING;
	}

	void SetToolbar(wxToolBar *toolbar)
	{
		m_toolbarWindow = toolbar;
		m_type = wxDOCKING_TOOLBAR;
	}

private:
	void init(wxString const &title = "");

private:
	union
	{
		wxWindow *m_userWindow;
		wxDockingPanel *m_dockingPanel;
		wxSplitterWindow *m_splitter;
		wxNotebook *m_notebook;
		wxDockingFrame *m_frame;
		wxToolBar *m_toolbarWindow;
	};
	wxDockingPanelType m_type;

	wxString m_title;

	wxDECLARE_EVENT_TABLE();

	// We don't allow copying because we need to keep track of the panels.
	wxDECLARE_DYNAMIC_CLASS_NO_COPY(wxDockingPanel);
};

#endif // wxUSE_DOCKING
