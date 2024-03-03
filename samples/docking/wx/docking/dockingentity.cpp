
#if wxUSE_DOCKING

#include <wx/object.h>
#include <wx/toolbar.h>

#include <wx/docking/dockingframe.h>

wxDockingEntityType wxDockingEntity::Set(wxWindow *window)
{
	if (!window)
	{
		m_window = window;
		m_type = wxDOCKING_NONE;
		return m_type;
	}

	{
		wxSplitterWindow *w = wxDynamicCast(window, wxSplitterWindow);
		if (w)
		{
			m_splitter = w;
			m_type = wxDOCKING_SPLITTER;

			return m_type;
		}
	}

	{
		wxNotebook *w = wxDynamicCast(window, wxNotebook);
		if (w)
		{
			m_notebook = w;
			m_type = wxDOCKING_NOTEBOOK;

			return m_type;
		}
	}

	{
		wxToolBar *w = wxDynamicCast(window, wxToolBar);
		if (w)
		{
			m_toolbar = w;
			m_type = wxDOCKING_TOOLBAR;

			return m_type;
		}
	}

	{
		wxDockingFrame *w = wxDynamicCast(window, wxDockingFrame);
		if (w)
		{
			m_frame = w;
			m_type = wxDOCKING_FRAME;

			return m_type;
		}
	}

	{
		//wxDockingPlaceholder *w = wxDynamicCast(window, wxDockingPlaceholder);
		wxDockingPlaceholder *w = dynamic_cast<wxDockingPlaceholder *>(window);
		if (w)
		{
			m_placeholder = w;
			m_type = wxDOCKING_PLACEHOLDER;

			return m_type;
		}
	}

	m_window = window;
	m_type = wxDOCKING_WINDOW;

	return m_type;
}

#endif // wxUSE_DOCKING
