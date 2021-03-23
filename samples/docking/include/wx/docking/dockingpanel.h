#ifndef _WX_DOCKINGPANEL_H_
#define _WX_DOCKINGPANEL_H_

#include "wx/defs.h"

#if wxUSE_DOCKING

#include <wx/panel.h>
#include <wx/splitter.h>
#include <wx/notebook.h>

#include <wx/docking/docking_defs.h>

class wxDockingFrame;
/**
 * wxDockingPanel represents a single view in the dockable window framework.
 * It can have the following states:
 *    - Floating: It shows at least one panel in a seperate window
 *    - Tabbed: It shows one or more panels
 *    - Splitted: It shows tww panels.
 * 
 * A single instance of wxDockingPanel, can only be splitted or tabbed.
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

	wxString const &getTitle(void) const { return m_title; }
	void setTitle(wxString const &title) { m_title = title;  }

	bool isFloating(void) const { return m_floating; }
	bool isSplitted(void) const { return m_splitted; }
	bool isTabbed(void) const { return m_tabbed; }
	bool isUser(void) const { return !m_tabbed && !m_splitted; }

protected:
	void OnSize(wxSizeEvent &event);

protected: // API

	wxWindow *getUserPanel(void) const { return (!m_tabbed && !m_splitted) ? m_panel : nullptr; }
	void setUserPanel(wxWindow *panel)
	{
		m_panel = panel;
		m_splitted = false;
		m_tabbed = false;
	}

	wxSplitterWindow *getSplitter(void) const { return (m_splitted) ? m_splitter : nullptr; }
	void setSplitter(wxSplitterWindow *splitter)
	{
		m_splitter = splitter;
		m_splitted = true;
		m_tabbed = false;
	}

	wxNotebook *getNotebook(void) const { return (m_tabbed) ? m_notebook : nullptr; }
	void setNotebook(wxNotebook *notebook)
	{
		m_notebook = notebook;
		m_splitted = false;
		m_tabbed = true;
	}

private:
	void init(wxString const &title = "");

private:
	union
	{
		wxWindow *m_panel;
		wxSplitterWindow *m_splitter;
		wxNotebook *m_notebook;
	};
	wxString m_title;
	bool m_floating : 1;
	bool m_splitted : 1;
	bool m_tabbed : 1;

	wxDECLARE_EVENT_TABLE();
	// We don't allow copying because we need to keep track of the panels.
	wxDECLARE_DYNAMIC_CLASS_NO_COPY(wxDockingPanel);
};

#endif // wxUSE_DOCKING

#endif // _WX_DOCKINGFRAME_H_
