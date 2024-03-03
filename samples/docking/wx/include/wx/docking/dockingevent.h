#ifndef _WX_DOCKING_EVENT_H_
#define _WX_DOCKING_EVENT_H_

#include <memory>

#include "wx/defs.h"

#if wxUSE_DOCKING

#include <wx/event.h>
#include <wx/eventfilter.h>

#include <wx/docking/docking_defs.h>
#include <wx/docking/dockinginfo.h>

class wxIDockingOverlay;

class WXDLLIMPEXP_CORE wxDockingEvent
: public wxNotifyEvent
{
public:
	wxDockingEvent(wxEventType type = wxEVT_NULL);
	wxDockingEvent(const wxDockingEvent &event)
	: wxNotifyEvent(event)
	{
		Assign(event);
	}

	~wxDockingEvent() wxOVERRIDE
	{
	}

	wxDockingEvent &operator=(const wxDockingEvent &event)
	{
		if (&event != this)
			Assign(event);

		return *this;
	}

	void Reset()
	{
		Assign(wxDockingEvent(wxEVT_NULL));
	}

	wxDockingFrame *GetFrame() const { return m_frame; }
	void SetFrame(wxDockingFrame *frame) { m_frame = frame; }

	wxWindow *GetWindowAtPoint() const { return m_windowAtPoint; }
	void SetWindowAtPoint(wxWindow *window) { m_windowAtPoint = window; }

	void SetSource(wxDockingInfo const &site) { m_src = site; }
	const wxDockingInfo &GetSource() const { return m_src; }
	wxDockingInfo &GetSource() { return m_src; }

	void SetTarget(wxDockingInfo const &site) { m_tgt = site; }
	const wxDockingInfo &GetTarget() const { return m_tgt; }
	wxDockingInfo &GetTarget() { return m_tgt; }

	void SetLastKnownTarget(wxDockingInfo const &target) { m_lastKnownTarget = target; }
	const wxDockingInfo &GetLastKnownTarget() const { return m_lastKnownTarget; }

	const wxPoint &GetEventPos() const { return m_eventPos; }
	wxPoint &GetEventPos() { return m_eventPos; }
	void SetEventPos(wxPoint const &coordinates) { m_eventPos = coordinates; }

	void SetDockingAllow(bool allow = true) { m_dockingAllow = allow; }
	bool IsDockingAllowed() const { return m_dockingAllow; }

public: // wxWdgets overrides
	wxDockingEvent *Clone() const wxOVERRIDE { return new wxDockingEvent(*this); }
	wxEventCategory GetEventCategory() const wxOVERRIDE { return wxEVT_CATEGORY_USER_INPUT; }

protected:
	void Assign(const wxDockingEvent &evt);

private:
	wxDockingFrame *m_frame;		// The current frame.
	wxWindow *m_windowAtPoint;

	// The source panel we potentially want to move in an docking event. This field
	// should never be modified by an event handler and is initialized only at the
	// start of a docking sequence.
	wxDockingInfo m_src;

	// This adresses the target which we potentially want to dock to. Eventhandler can
	// safely modify this as needed.
	wxDockingInfo m_tgt;

	// Stores the lastknown target. As long as the user targets a valid panel, this is
	// the same as tgt. If the user moves the mouse outside the applications or an overlay
	// presents some windowsfor i.E. navigation then no valid docking panel can be identified
	// and in this case the last known target is still available.
	// Eventhandlers should not modify this, as it is only available for reference.
	wxDockingInfo m_lastKnownTarget;

	wxPoint m_eventPos;				// Screencoordinates of the mouse for this event

	bool m_dockingAllow:1;			// Flag if docking is allowed at the target.

	wxDECLARE_DYNAMIC_CLASS(wxDockingEvent);
};

class WXDLLIMPEXP_DOCKING wxDockingEventFilter
: public wxEventFilter
{
public:
	wxDockingEventFilter();
	~wxDockingEventFilter() wxOVERRIDE;

	int FilterEvent(wxEvent &event) wxOVERRIDE;
};

typedef void (wxEvtHandler:: *wxDockingEventFunction)(wxDockingEvent &);

#define wxDockingEventHandler(func)                 wxEVENT_HANDLER_CAST(wxDockingEventFunction, func)
#define wx__DECLARE_DOCKINGEVT(evt, fn)             wx__DECLARE_EVT0(wxEVT_DOCKING_ ## evt, wxDockingEventHandler(fn))

#define EVT_DOCKING_START(fn)                       wx__DECLARE_DOCKINGEVT(START, fn)				// Can be vetoed. If vetoed, then dragging is disabled until the user initiates the next START event.
#define EVT_DOCKING_TRACK(fn)                       wx__DECLARE_DOCKINGEVT(TRACK, fn)				// Can not be vetoed, as the user drags the mouse around. The event determines if a target can be selected.
#define EVT_DOCKING_HINT(fn)                        wx__DECLARE_DOCKINGEVT(HINT, fn)				// Can not be vetoed. The event is responsible for updating the GUI so the user knows where he can dock to.
#define EVT_DOCKING_END(fn)                         wx__DECLARE_DOCKINGEVT(END, fn)					// Can not be vetoed. When the user releases the mouse, that's it.
#define EVT_DOCKING_CREATE_PANEL(fn)                wx__DECLARE_DOCKINGEVT(CREATE_PANEL, fn)		// Can not be vetoed. Request for creating an appropriate (custom) panel must be handled.
#define EVT_DOCKING_RELEASE_PANEL(fn)               wx__DECLARE_DOCKINGEVT(RELEASE_PANEL, fn)		// Can not be vetoed. The event is responsible for removal of the panel. Default is to destroy it, but custom code can do whatever it wants (like cachingfor later reuse).
#define EVT_DOCKING_TRY_REMOVE_PANEL(fn)            wx__DECLARE_DOCKINGEVT(TRY_REMOVE_PANEL, fn)	// Can be vetoed. If all technical requirements are met, and the target is a valid docking target, the even can override the decision to deny a docking request.
																									// If the technical requirements for a target are not met, this event is not sent.
#define EVT_DOCKING_MOVE_PANEL(fn)                  wx__DECLARE_DOCKINGEVT(MOVE_PANEL, fn)			// Can not be vetoed. The panel is moved from the source to the target.

wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_DOCKING_START, wxDockingEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_DOCKING_TRACK, wxDockingEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_DOCKING_HINT, wxDockingEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_DOCKING_END, wxDockingEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_DOCKING_CREATE_PANEL, wxDockingEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_DOCKING_RELEASE_PANEL, wxDockingEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_DOCKING_TRY_REMOVE_PANEL, wxDockingEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_DOCKING_MOVE_PANEL, wxDockingEvent);

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_EVENT_H_
