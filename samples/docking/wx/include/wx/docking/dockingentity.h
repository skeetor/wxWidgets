#ifndef _WX_DOCKING_ENTITY_H_
#define _WX_DOCKING_ENTITY_H_

#include "wx/defs.h"

#if wxUSE_DOCKING

#include <wx/toolbar.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include <wx/docking/docking_defs.h>
#include <wx/docking/dockingutils.h>

#ifndef wxUSE_NOTEBOOK
#error wxDockingFrame requires wxNotebook enabled
#endif

#ifndef wxUSE_SPLITTER
#error wxDockingFrame requires wxSplitterWindow enabled
#endif

class WXDLLIMPEXP_DOCKING wxDockingPlaceholder
: public wxWindow
{
public:
	wxDockingPlaceholder(wxWindow *parent)
	: wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN|wxSTATIC_BORDER, wxASCII_STR(wxPanelNameStr))
	{
	}

	~wxDockingPlaceholder() override
	{
	}
};

class WXDLLIMPEXP_DOCKING wxDockingEntity
{
public:
	wxDockingEntity()
	: m_window(nullptr)
	, m_type(wxDOCKING_NONE)
	{
	}

	wxDockingEntity(wxWindow *window)
	: m_window(nullptr)
	, m_type(wxDOCKING_NONE)
	{
		if (window)
			Set(window);
	}

	wxDockingEntity(wxWindow *window, wxDockingEntityType type)
	: m_window(window)
	, m_type(type)
	{
	}

	wxWindow *operator->() const { return m_window; }
    operator wxWindow *() const { return m_window; }

	bool operator ==(wxWindow *window) const { return window == m_window; }
	bool operator ==(wxDockingEntity const &pp) const { return pp.GetWindow() == m_window; }

	wxDockingEntityType GetType() const { return m_type; }
	void SetType(wxDockingEntityType type) { m_type = type; }

    static wxDockingEntityType CheckType(wxWindow *window) { wxDockingEntity w(window); return w.GetType(); }

    wxDockingEntityType Set(wxWindow *window);
    void Set(wxWindow *window, wxDockingEntityType type) { m_window = window; m_type = type; }

	wxWindow *GetWindow() const { return m_window; }
	void SetWindow(wxWindow *window) { m_window = window; (window == nullptr) ? m_type = wxDOCKING_NONE : m_type = wxDOCKING_USER; }

	wxNotebook *GetNotebook() const { return (m_type == wxDOCKING_NOTEBOOK) ? m_notebook : nullptr; }
	void SetNotebook(wxNotebook *window) { m_notebook = window; (window == nullptr) ? m_type = wxDOCKING_NONE : m_type = wxDOCKING_NOTEBOOK; }

	wxSplitterWindow *GetSplitter() const { return (m_type == wxDOCKING_SPLITTER) ? m_splitter : nullptr; }
	void SetSplitter(wxSplitterWindow *window) { m_splitter = window; (window == nullptr) ? m_type = wxDOCKING_NONE : m_type = wxDOCKING_SPLITTER; }

	wxToolBar *GetToolBar() const { return (m_type == wxDOCKING_TOOLBAR) ? m_toolbar : nullptr; }
	void SetToolBar(wxToolBar *window) { m_toolbar = window; (window == nullptr) ? m_type = wxDOCKING_NONE : m_type = wxDOCKING_TOOLBAR; }

	wxDockingFrame *GetFrame() const { return (m_type == wxDOCKING_FRAME) ? m_frame : nullptr; }
	void SetFrame(wxDockingFrame *window) { m_frame = window; (window == nullptr) ? m_type = wxDOCKING_NONE : m_type = wxDOCKING_FRAME; }

	wxDockingPlaceholder *GetPlaceholder() const { return (m_type == wxDOCKING_PLACEHOLDER) ? m_placeholder : nullptr; }
	void SetPlaceholder(wxDockingPlaceholder *window) { m_placeholder = window; (window == nullptr) ? m_type = wxDOCKING_NONE : m_type = wxDOCKING_PLACEHOLDER; }

private:
	union
	{
		wxWindow			*m_window;
		wxSplitterWindow	*m_splitter;
		wxNotebook			*m_notebook;
		wxToolBar			*m_toolbar;
		wxDockingFrame		*m_frame;
		wxDockingPlaceholder *m_placeholder;
	};

	wxDockingEntityType m_type:16;
};


class WXDLLIMPEXP_DOCKING wxDockingEntityState
: public wxDockingEntity
{
public:
	wxDockingEntityState()
		: wxDockingEntity()
		, m_locked(false)
		, m_draggable(true)
	{
	}

	wxDockingEntityState(wxWindow *window)
		: wxDockingEntity(window)
		, m_locked(false)
		, m_draggable(true)
	{
	}

	wxDockingEntityState(wxDockingEntity const &panel)
		: wxDockingEntity(panel)
		, m_locked(false)
		, m_draggable(true)
	{
	}

	wxDockingEntityState(wxWindow *window, wxDockingEntityType type)
		: wxDockingEntity(window, type)
		, m_locked(false)
		, m_draggable(true)
	{
	}

	operator wxDockingEntity const &() const { return *this; }

	/**
	 * Locking a panel means that it will not be removed when it is automatically to be removed because it is no longer needed.
	 * This will not affect removal if RemovePanel is called. Note that this only affects docking containers like splitter, notebook
	 * or frames. It is not needed for user panels, as those are handled by client code.
	 * Preventing a panel to be removed can also be done by vetoing the EVT_DOCKING_TRY_REMOVE_PANEL event.
	 * If lock is false the panel is unlocked if it was locked before.
	 */
	void SetLocked(bool lock = true) { m_locked = lock; }
	bool IsLocked() const { return m_locked; }

	/**
	 * Enables or disables dragging for the given panel. By default, a panel is always draggable, so it doesn't need to be used on
	 * all panels. If a particular panel should not be movable, then it can be disabled, so it can not be dragged by the user anymore.
	 * This can also be achieved by vetoing the EVT_BEGIN event when the source window should not be dragged. If a panel is disabled
	 * for dragging, then there will be no events when this window is the source.
	 */
	void SetDraggable(bool draggable = true) { m_draggable = draggable; }
	bool IsDraggable() const { return m_draggable; }

	void SetTitle(wxString const &title) { m_title = title; }
	wxString const &GetTitle() const { return m_title; }

private:
	bool m_locked:1;
	bool m_draggable : 1;
	wxString m_title;
};

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_ENTITY_H_
