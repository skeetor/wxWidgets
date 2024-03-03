
#if wxUSE_DOCKING

#include <wx/docking/dockingevent.h>
#include <wx/docking/dockingframe.h>

#if wxUSE_GUI
wxIMPLEMENT_DYNAMIC_CLASS(wxDockingEvent, wxEvent);
#endif // wxUSE_GUI

wxDEFINE_EVENT(wxEVT_DOCKING_START, wxDockingEvent);
wxDEFINE_EVENT(wxEVT_DOCKING_TRACK, wxDockingEvent);
wxDEFINE_EVENT(wxEVT_DOCKING_END, wxDockingEvent);
wxDEFINE_EVENT(wxEVT_DOCKING_CREATE_PANEL, wxDockingEvent);
wxDEFINE_EVENT(wxEVT_DOCKING_RELEASE_PANEL, wxDockingEvent);
wxDEFINE_EVENT(wxEVT_DOCKING_ADD_PANEL, wxDockingEvent);
wxDEFINE_EVENT(wxEVT_DOCKING_TRY_REMOVE_PANEL, wxDockingEvent);
wxDEFINE_EVENT(wxEVT_DOCKING_REMOVE_PANEL, wxDockingEvent);
wxDEFINE_EVENT(wxEVT_DOCKING_MOVE_PANEL, wxDockingEvent);
wxDEFINE_EVENT(wxEVT_DOCKING_HINT, wxDockingEvent);

wxDockingEvent::wxDockingEvent(wxEventType type)
: wxNotifyEvent(type)
, m_frame(nullptr)
, m_dockingAllow(true)
, m_hasOverlayHandler(false)
{
}

void wxDockingEvent::Assign(const wxDockingEvent &event)
{
	wxEvent::operator=(event);

	m_frame = event.m_frame;
	m_windowAtPoint = event.m_windowAtPoint;
	m_src = event.m_src;
	m_tgt = event.m_tgt;
	m_eventPos = event.m_eventPos;
	m_dockingAllow = event.m_dockingAllow;
	m_hasOverlayHandler = event.m_hasOverlayHandler;
	m_overlayHandler = event.m_overlayHandler;
}

wxDockingEventFilter::wxDockingEventFilter()
{
}

wxDockingEventFilter::~wxDockingEventFilter()
{
}

int wxDockingEventFilter::FilterEvent(wxEvent &event)
{
	// We want to be sure that the frame for the given window is used. The client can
	// have multiple frame windows (for example when a window is floating), so we have
	// to make sure that the event is propagated to the associated frame.
	const wxEventType t = event.GetEventType();
	if (t == wxEVT_LEFT_DOWN)
	{
		wxWindow *w = wxDynamicCast(event.GetEventObject(), wxWindow);
		wxDockingFrame *frame = wxDockingUtils::DockingFrameFromWindow(w);
		if (frame)
			return frame->OnMouseLeftDown((wxMouseEvent &)event);
	}
	if (t == wxEVT_LEFT_UP)
	{
		wxWindow *w = wxDynamicCast(event.GetEventObject(), wxWindow);
		wxDockingFrame *frame = wxDockingUtils::DockingFrameFromWindow(w);
		if (frame)
			return frame->OnMouseLeftUp((wxMouseEvent &)event);
	}
	else if (t == wxEVT_MOTION)
	{
		wxWindow *w = wxDynamicCast(event.GetEventObject(), wxWindow);
		wxDockingFrame *frame = wxDockingUtils::DockingFrameFromWindow(w);
		if (frame)
			return frame->OnMouseMove((wxMouseEvent &)event);
	}

	return wxEventFilter::Event_Skip;	// Continue regular processing
}

#endif // wxUSE_DOCKING
